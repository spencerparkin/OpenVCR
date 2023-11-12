#include "FileAudioSource.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

FileAudioSource::FileAudioSource(const std::string& givenName) : AudioDevice(givenName)
{
	this->audioFilePath = new std::string();
	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;
	this->audioSinkName = new std::string();
	this->futurePosition = 0;
	this->futureBufferSize = 10 * 1024;
	this->nextSampleBuffer = new std::vector<Uint8>();
}

/*virtual*/ FileAudioSource::~FileAudioSource()
{
	delete this->audioFilePath;
	delete this->audioSinkName;
	delete this->nextSampleBuffer;
}

/*static*/ FileAudioSource* FileAudioSource::Create(const std::string& name)
{
	return new FileAudioSource(name);
}

/*virtual*/ bool FileAudioSource::PowerOn(Machine* machine, Error& error)
{
	if (!SDL_LoadWAV(this->audioFilePath->c_str(), &this->audioSpec, &this->audioBuffer, &this->audioBufferSize))
	{
		error.Add(SDL_GetError());
		return false;
	}

	AudioDevice* audioSinkDevice = machine->FindIODevice<AudioDevice>(*this->audioSinkName);
	if (!audioSinkDevice)
	{
		error.Add("Can't generate audio samples if no audio sink device was given.");
		return false;
	}

	this->futurePosition = 0;
	return true;
}

/*virtual*/ bool FileAudioSource::PowerOff(Machine* machine, Error& error)
{
	SDL_FreeWAV(this->audioBuffer);

	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;

	return true;
}

/*virtual*/ bool FileAudioSource::MoveData(Machine* machine, Error& error)
{
	AudioDevice* audioSinkDevice = machine->FindIODevice<AudioDevice>(*this->audioSinkName);
	if (!audioSinkDevice)
	{
		error.Add("Can't move sample data if no audio sink specified.");
		return false;
	}

	Uint32 playbackPosition = audioSinkDevice->GetPlaybackPosition();

	if (machine->GetDisposition() == Machine::Disposition::PLACE)
	{
		double position = machine->GetPosition();

		// TODO: If the machine position drifts away from the playback position by more than a given tolerance, then adjust the playback position.
		//       This means calling audioSinkDevice->SetPlaybackPosition() which will disrupt audio, but maybe this won't happen too often
		//       if the machine position is being moved along at a close-enough speed to the playback rate and we have a large-enough tolerance?
	}

	Uint32 numBytesToGrab = 0;
	if (futurePosition <= playbackPosition)
	{
		futurePosition = playbackPosition;
		numBytesToGrab = futureBufferSize;
	}
	else
	{
		Uint32 numBytesBuffered = futurePosition - playbackPosition;
		if (numBytesBuffered < this->futureBufferSize)
			numBytesToGrab = this->futureBufferSize - numBytesBuffered;
	}

	if (futurePosition + numBytesToGrab > this->audioBufferSize)
		numBytesToGrab = this->audioBufferSize - futurePosition;

	this->nextSampleBuffer->clear();
	this->nextSampleBuffer->resize(numBytesToGrab);
	::memcpy(this->nextSampleBuffer->data(), &this->audioBuffer[futurePosition], numBytesToGrab);

	futurePosition += numBytesToGrab;

	this->complete = true;
	return true;
}

/*virtual*/ bool FileAudioSource::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	sampleBuffer.clear();
	sampleBuffer.resize(this->nextSampleBuffer->size());
	::memcpy(sampleBuffer.data(), this->nextSampleBuffer->data(), this->nextSampleBuffer->size());
	return sampleBuffer.size() > 0;
}

void FileAudioSource::SetAudioFilePath(const std::string& audioFilePath)
{
	*this->audioFilePath = audioFilePath;
}

const std::string& FileAudioSource::GetAudioFilePath() const
{
	return *this->audioFilePath;
}

void FileAudioSource::SetAudioSinkName(const std::string& audioSinkName)
{
	*this->audioSinkName = audioSinkName;
}

const std::string& FileAudioSource::GetAudioSinkName() const
{
	return *this->audioSinkName;
}