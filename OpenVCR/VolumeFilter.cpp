#include "VolumeFilter.h"
#include "Machine.h"
#include "Error.h"

using namespace OpenVCR;

VolumeFilter::VolumeFilter(const std::string& givenName) : AudioDevice(givenName)
{
	this->volumeFactor = 1.0;
	this->filteredSampleBuffer = new std::vector<Uint8>();
}

/*virtual*/ VolumeFilter::~VolumeFilter()
{
	delete this->filteredSampleBuffer;
}

/*static*/ VolumeFilter* VolumeFilter::Create(const std::string& name)
{
	return new VolumeFilter(name);
}

/*virtual*/ int VolumeFilter::GetSortKey() const
{
	return 1;
}

/*virtual*/ bool VolumeFilter::PowerOn(Machine* machine, Error& error)
{
	AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(*this->sourceName);
	if (!audioDevice)
	{
		error.Add("Volume filter needs source device.");
		return false;
	}

	::memcpy(&this->audioSpec, audioDevice->GetAudioSpec(), sizeof(SDL_AudioSpec));
	return true;
}

/*virtual*/ bool VolumeFilter::PowerOff(Machine* machine, Error& error)
{
	return true;
}

/*virtual*/ bool VolumeFilter::MoveData(Machine* machine, Error& error)
{
	AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(*this->sourceName);
	if (!audioDevice)
	{
		error.Add("Volume filter needs source device.");
		return false;
	}

	if (!audioDevice->IsComplete())
		return true;

	this->filteredSampleBuffer->clear();
	if (audioDevice->GetSampleData(*this->filteredSampleBuffer))
	{
		Uint32 bytesPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format) / 8;

		// We need to check this so that we make sure we know where one sample begins and another ends.
		if (this->filteredSampleBuffer->size() % bytesPerSample != 0)
		{
			error.Add("Volume filter needs sample buffer to be a multiple of the sample size.");
			return false;
		}

		if (bytesPerSample == 2 && SDL_AUDIO_ISSIGNED(this->audioSpec.format))
		{
			Sint16* sampleArray = reinterpret_cast<Sint16*>(this->filteredSampleBuffer->data());
			Uint32 sampleArraySize = Uint32(this->filteredSampleBuffer->size()) / bytesPerSample;
			for (int i = 0; i < (signed)sampleArraySize; i++)
				sampleArray[i] = Sint16(double(sampleArray[i]) * this->volumeFactor);
		}
		else
		{
			error.Add("Volume filter does not yet support incoming audio format.");
			return false;
		}
	}

	this->complete = true;
	return true;
}

/*virtual*/ bool VolumeFilter::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	sampleBuffer.clear();
	sampleBuffer.resize(this->filteredSampleBuffer->size());
	::memcpy(sampleBuffer.data(), this->filteredSampleBuffer->data(), this->filteredSampleBuffer->size());
	return sampleBuffer.size() > 0;
}

void VolumeFilter::SetVolume(double volume)
{
	if (volume < 0.0)
		volume = 0.0;
	else if (volume > 1.0)
		volume = 1.0;

	this->volumeFactor = volume;
}

double VolumeFilter::GetVolume() const
{
	return this->volumeFactor;
}