#include "FileAudioSource.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

FileAudioSource::FileAudioSource(const std::string& givenName) : AudioDevice(givenName)
{
	this->totalDurationSeconds = 0.0;
	this->playbackDriftToleranceSeconds = 3.0;
	this->futureBufferSeconds = 5.0;
	this->audioFilePath = new std::string();
	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;
	this->audioSinkName = new std::string();
	this->futurePosition = 0;
	this->sampleChunkSizeBytes = 16;
	this->nextSampleOffset = 0;
}

/*virtual*/ FileAudioSource::~FileAudioSource()
{
	delete this->audioFilePath;
	delete this->audioSinkName;
}

/*static*/ FileAudioSource* FileAudioSource::Create(const std::string& name)
{
	return new FileAudioSource(name);
}

/*virtual*/ bool FileAudioSource::PowerOn(Machine* machine, Error& error)
{
	// If the WAVE file is too big to be fully resident, then it's probably just not practical anyway.  I'm not going to worry about streaming from disk.
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

	this->totalDurationSeconds = this->AudioBufferOffsetToTimeSeconds(this->audioBufferSize);

	this->futurePosition = 0;
	audioSinkDevice->SetPlaybackTime(0.0);
	this->poweredOn = true;
	return true;
}

/*virtual*/ bool FileAudioSource::PowerOff(Machine* machine, Error& error)
{
	SDL_FreeWAV(this->audioBuffer);

	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;

	this->poweredOn = false;
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

	double actualPlaybackTimeSeconds = 0.0;
	audioSinkDevice->GetPlaybackTime(actualPlaybackTimeSeconds);

	if (machine->GetDisposition() == Machine::Disposition::PLACE)
	{
		double position = machine->GetPosition();
		double desiredPlaybackTimeSeconds = position * this->totalDurationSeconds;

		if (fabs(desiredPlaybackTimeSeconds - actualPlaybackTimeSeconds) > this->playbackDriftToleranceSeconds)
		{
			this->futurePosition = this->AudioBufferOffsetFromTimeSeconds(desiredPlaybackTimeSeconds);
			audioSinkDevice->SetPlaybackTime(desiredPlaybackTimeSeconds);
		}
	}

	double futureTimeSeconds = this->AudioBufferOffsetToTimeSeconds(this->futurePosition);
	if (futureTimeSeconds - actualPlaybackTimeSeconds < this->futureBufferSeconds)
	{
		this->nextSampleOffset = this->futurePosition;
		this->futurePosition += this->sampleChunkSizeBytes;
	}
	else
	{
		this->nextSampleOffset = -1;
	}

	this->complete = true;
	return true;
}

/*virtual*/ bool FileAudioSource::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	sampleBuffer.clear();
	
	if (this->nextSampleOffset != -1)
	{
		for (int i = this->nextSampleOffset; i < (signed)(this->nextSampleOffset + this->sampleChunkSizeBytes); i++)
			if (0 <= i && i < (signed)this->audioBufferSize)
				sampleBuffer.push_back(this->audioBuffer[i]);
	}

	return sampleBuffer.size() > 0;
}

double FileAudioSource::AudioBufferOffsetToTimeSeconds(Uint32 audioBufferOffset) const
{
	Uint32 bytesPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format) / 8;
	Uint32 sampleFrameSize = this->audioSpec.channels * bytesPerSample;
	return double(audioBufferOffset) / (double(sampleFrameSize) * double(this->audioSpec.freq));
}

// The returned offset here is guarenteed to be on a sample-frame boundary.
Uint32 FileAudioSource::AudioBufferOffsetFromTimeSeconds(double timeSeconds) const
{
	Uint32 bytesPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format) / 8;
	Uint32 sampleFrameSize = this->audioSpec.channels * bytesPerSample;
	Uint32 audioBufferOffset = Uint32(timeSeconds * double(sampleFrameSize) * double(this->audioSpec.freq));
	Uint32 remainder = audioBufferOffset % sampleFrameSize;
	audioBufferOffset -= remainder;
	return audioBufferOffset;
}

/*virtual*/ std::string FileAudioSource::GetStatusMessage() const
{
	double futureTimeSeconds = this->AudioBufferOffsetToTimeSeconds(this->futurePosition);
	return std::format("Future time: {:.2f}", futureTimeSeconds);
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