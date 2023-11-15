#include "AdditiveAudioMixer.h"
#include "Machine.h"
#include "Error.h"

using namespace OpenVCR;

AdditiveAudioMixer::AdditiveAudioMixer(const std::string& givenName) : AudioDevice(givenName)
{
	this->mixedAudioSample = new std::vector<Uint8>();
	this->byteStreamArray = new std::vector<RawByteStream*>();
}

/*virtual*/ AdditiveAudioMixer::~AdditiveAudioMixer()
{
	delete this->mixedAudioSample;
	delete this->byteStreamArray;
}

/*static*/ AdditiveAudioMixer* AdditiveAudioMixer::Create(const std::string& name)
{
	return new AdditiveAudioMixer(name);
}

/*virtual*/ bool AdditiveAudioMixer::PowerOn(Machine* machine, Error& error)
{
	if (this->byteStreamArray->size() > 0)
	{
		error.Add("Byte stream array non-zero!");
		return false;
	}

	if (this->GetNumSourceNames() < 2)
	{
		error.Add("Additive audio mixer expected 2 or more source names.");
		return false;
	}

	::memset(&this->audioSpec, 0, sizeof(this->audioSpec));

	for (int i = 0; i < this->GetNumSourceNames(); i++)
	{
		AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(this->GetSourceName(i));
		if (!audioDevice)
		{
			error.Add(std::format("Failed to get audio source \"{}\".", this->GetSourceName(i).c_str()));
			return false;
		}

		if (this->audioSpec.format == 0)
			::memcpy(&this->audioSpec, audioDevice->GetAudioSpec(), sizeof(SDL_AudioSpec));
		else if (memcmp(&this->audioSpec, audioDevice->GetAudioSpec(), sizeof(SDL_AudioSpec)) != sizeof(SDL_AudioSpec))
		{
			error.Add("All audio sources of the additive mixer must have the same audio format.");
			return false;
		}

		this->byteStreamArray->push_back(new RawByteStream());
	}

	this->poweredOn = true;
	return true;
}

/*virtual*/ bool AdditiveAudioMixer::PowerOff(Machine* machine, Error& error)
{
	for (RawByteStream* byteStream : *this->byteStreamArray)
		delete byteStream;

	this->byteStreamArray->clear();

	this->poweredOn = false;
	return true;
}

/*virtual*/ bool AdditiveAudioMixer::MoveData(Machine* machine, Error& error)
{
	// First, make sure all our inputs are ready.
	for (int i = 0; i < this->GetNumSourceNames(); i++)
	{
		AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(this->GetSourceName(i));
		if (!audioDevice)
		{
			error.Add(std::format("Failed to get audio source {}.", i));
			return false;
		}

		if (!audioDevice->IsComplete())
			return true;
	}

	// Pull sample data from all the inputs.
	for (int i = 0; i < this->GetNumSourceNames(); i++)
	{
		AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(this->GetSourceName(i));

		std::vector<Uint8> sampleData;
		if (audioDevice->GetSampleData(sampleData))
		{
			RawByteStream* byteStream = (*this->byteStreamArray)[i];
			byteStream->AddBytes(sampleData.data(), (Uint32)sampleData.size());
		}
	}

	// What's the biggest chunk we can bite out of all streams?
	Uint32 maxSize = std::numeric_limits<Uint32>::max();
	for (RawByteStream* byteStream : *this->byteStreamArray)
	{
		Uint32 numBytesAvailable = byteStream->NumBytesAvailable();
		if (numBytesAvailable < maxSize)
			maxSize = numBytesAvailable;
	}

	// Now go mix the streams to prepare our output.
	this->mixedAudioSample->clear();
	this->mixedAudioSample->resize(maxSize);
	for (RawByteStream* byteStream : *this->byteStreamArray)
	{
		std::vector<Uint8> audioSample;
		audioSample.resize(maxSize);
		Uint32 audioSampleSize = 0;
		byteStream->RemoveBytes(audioSample.data(), audioSampleSize);
		if (audioSampleSize != maxSize)
		{
			error.Add(std::format("Failed to grab {} bytes from byte stream.", maxSize));
			return false;
		}

		if (!this->IntegrateSample(audioSample, error))
		{
			error.Add("Failed to integrate audio sample.");
			return false;
		}
	}

	this->complete = true;
	return true;
}

bool AdditiveAudioMixer::IntegrateSample(const std::vector<Uint8>& audioSample, Error& error)
{
	Uint32 bytesPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format) / 8;
	if (audioSample.size() % bytesPerSample != 0)
	{
		error.Add("Sample buffer needs to be a multiple of the sample size for integration to work.");
		return false;
	}

	Uint32 numSamples = (Uint32)audioSample.size() / bytesPerSample;

	if (SDL_AUDIO_ISSIGNED(this->audioSpec.format))
	{
		// TODO: May need to account for endianness one day.
		switch (bytesPerSample)
		{
			case 2:
			{
				const Sint16* sampleArray = reinterpret_cast<const Sint16*>(audioSample.data());
				Sint16* mixedSampleArray = reinterpret_cast<Sint16*>(this->mixedAudioSample->size());
				
				// Note that this all works independant of how many channels there are.
				for (Uint32 i = 0; i < numSamples; i++)
					mixedSampleArray[i] += sampleArray[i];

				return true;
			}
		}
	}

	error.Add("Audio spec not yet supported in audio sample integrator.");
	return false;
}

/*virtual*/ bool AdditiveAudioMixer::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	sampleBuffer.resize(this->mixedAudioSample->size());
	::memcpy(sampleBuffer.data(), this->mixedAudioSample->data(), this->mixedAudioSample->size());
	return sampleBuffer.size() > 0;
}