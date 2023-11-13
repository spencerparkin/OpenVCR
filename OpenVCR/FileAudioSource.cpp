#include "FileAudioSource.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

FileAudioSource::FileAudioSource(const std::string& givenName) : AudioDevice(givenName)
{
	this->totalDurationSeconds = 0.0;
	this->playbackDriftToleranceSeconds = 3.0;
	this->audioFilePath = new std::string();
	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;
	this->audioSinkName = new std::string();
	this->playbackPosition = 0;
	this->playbackChunkSizeBytes = 16;
	this->nextSampleStart = 0;
	this->nextSampleEnd = 0;
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
	// TODO: We may need to be able to stream the audio from disk so that we don't need to have it all in RAM all at once.
	//       Each tick, we'd load some future audio, and only make sure the future is loaded up to a certain point.
	//       If ever the playback position had to be adjusted, we could wipe our cache and start over.
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

	Uint32 bytesPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format) / 8;
	this->totalDurationSeconds = double((this->audioBufferSize / bytesPerSample) / this->audioSpec.channels) / double(this->audioSpec.freq);

	this->playbackPosition = 0;
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

	if (machine->GetDisposition() == Machine::Disposition::PLACE)
	{
		double position = machine->GetPosition();
		double desiredPlaybackTimeSeconds = position * this->totalDurationSeconds;

		double actualPlaybackTimeSeconds = 0.0;
		audioSinkDevice->GetPlaybackTime(actualPlaybackTimeSeconds);

		if (fabs(desiredPlaybackTimeSeconds - actualPlaybackTimeSeconds) > this->playbackDriftToleranceSeconds)
		{
			// TODO: Calculate new playbackPosition to be on a sample-frame boundary.

			audioSinkDevice->SetPlaybackTime(desiredPlaybackTimeSeconds);
		}
	}

	// TODO: All this needs to be reworked so that we're only staying, say, a few seconds ahead of the sink.
	//       It would be useful to be able to convert from a buffer position to a time index, and vice-versa.
	this->nextSampleStart = this->playbackPosition;
	this->nextSampleEnd = this->playbackPosition + this->playbackChunkSizeBytes;

	this->playbackPosition += this->playbackChunkSizeBytes;

	this->complete = true;
	return true;
}

/*virtual*/ bool FileAudioSource::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	sampleBuffer.clear();
	for (int i = this->nextSampleStart; i < (signed)this->nextSampleEnd; i++)
		if (0 <= i && i < (signed)this->audioBufferSize)
			sampleBuffer.push_back(this->audioBuffer[i]);

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