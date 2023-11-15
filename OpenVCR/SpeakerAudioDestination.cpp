#include "SpeakerAudioDestination.h"
#include "Machine.h"

using namespace OpenVCR;

SpeakerAudioDestination::SpeakerAudioDestination(const std::string& givenName) : AudioDevice(givenName)
{
	this->deviceID = 0;
	this->deviceSubStr = new std::string();
	this->machineThreadBuffer = new std::vector<Uint8>();
	this->audioStream = nullptr;
	this->initialPlaybackTimeSeconds = 0.0;
	this->totalBytesSunk = 0;
	this->selectionCallback = new DeviceSelectionCallback;
}

/*virtual*/ SpeakerAudioDestination::~SpeakerAudioDestination()
{
	delete this->deviceSubStr;
	delete this->machineThreadBuffer;
	delete this->selectionCallback;
}

/*static*/ SpeakerAudioDestination* SpeakerAudioDestination::Create(const std::string& name)
{
	return new SpeakerAudioDestination(name);
}

/*virtual*/ bool SpeakerAudioDestination::PowerOn(Machine* machine, Error& error)
{
	if (this->deviceID != 0)
	{
		error.Add("Audio device ID already non-zero!");
		return false;
	}

	AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(*this->sourceName);
	if (!audioDevice)
	{
		error.Add("Can't power-on speaker audio destination without source audio device.");
		return false;
	}

	SDL_AudioSpec* sourceSpec = audioDevice->GetAudioSpec();
	SDL_AudioSpec desiredSpec;
	::memcpy(&desiredSpec, sourceSpec, sizeof(SDL_AudioSpec));
	desiredSpec.callback = &SpeakerAudioDestination::AudioCallbackEntry;
	desiredSpec.userdata = this;

	std::string chosenDevice;

	if (*this->selectionCallback)
	{
		if (!this->SelectAudioDevice(chosenDevice, *this->selectionCallback, DeviceType::OUTPUT, error))
			return false;
	}
	else if (this->deviceSubStr->length() > 0)
	{
		if (!this->SelectAudioDevice(chosenDevice, *this->deviceSubStr, DeviceType::OUTPUT, error))
			return false;
	}

	const char* audioDeviceName = nullptr;
	if (chosenDevice.length() > 0)
		audioDeviceName = chosenDevice.c_str();

	this->deviceID = SDL_OpenAudioDevice(audioDeviceName, 0, &desiredSpec, &this->audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (this->deviceID == 0)
	{
		error.Add(std::format("Failed to open audio device: {}", SDL_GetError()));
		return false;
	}

	this->audioStream = SDL_NewAudioStream(
		sourceSpec->format,
		sourceSpec->channels,
		sourceSpec->freq,
		this->audioSpec.format,
		this->audioSpec.channels,
		this->audioSpec.freq);
	if (!this->audioStream)
	{
		error.Add(std::format("Failed to create audio stream: {}", SDL_GetError()));
		return false;
	}

	this->initialPlaybackTimeSeconds = 0.0;
	this->totalBytesSunk = 0;
	SDL_PauseAudioDevice(this->deviceID, 0);

	this->poweredOn = true;
	return true;
}

/*virtual*/ bool SpeakerAudioDestination::PowerOff(Machine* machine, Error& error)
{
	if (this->deviceID != 0)
	{
		SDL_CloseAudioDevice(this->deviceID);
		this->deviceID = 0;
	}

	if (this->audioStream)
	{
		SDL_FreeAudioStream(this->audioStream);
		this->audioStream = nullptr;
	}

	this->poweredOn = false;
	return true;
}

void SpeakerAudioDestination::SetDeviceSubString(const std::string& deviceSubStr)
{
	*this->deviceSubStr = deviceSubStr;
}

void SpeakerAudioDestination::SetDeviceSelectionCallback(DeviceSelectionCallback selectionCallback)
{
	*this->selectionCallback = selectionCallback;
}

/*virtual*/ bool SpeakerAudioDestination::MoveData(Machine* machine, Error& error)
{
	AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(*this->sourceName);
	if (!audioDevice)
	{
		error.Add("Can't move data to speaker audio destination without source audio device.");
		return false;
	}

	if (!audioDevice->IsComplete())
		return true;

	// TODO: A volume control could/should be implemented using an audio filter put into the graph.

	this->machineThreadBuffer->clear();
	audioDevice->GetSampleData(*this->machineThreadBuffer);

	if (this->machineThreadBuffer->size() > 0)
	{
		SDL_LockAudioDevice(this->deviceID);
		SDL_AudioStreamPut(this->audioStream, this->machineThreadBuffer->data(), (int)this->machineThreadBuffer->size());
		SDL_UnlockAudioDevice(this->deviceID);
	}

	this->complete = true;
	return true;
}

void SpeakerAudioDestination::AudioCallback(Uint8* buffer, int length)
{
	int numBytesGotten = SDL_AudioStreamGet(this->audioStream, buffer, length);
	for (int i = numBytesGotten; i < length; i++)
		buffer[i] = this->audioSpec.silence;

	this->totalBytesSunk += numBytesGotten;
}

/*static*/ void SDLCALL SpeakerAudioDestination::AudioCallbackEntry(void* userData, Uint8* buffer, int length)
{
	auto speakerAudioDestination = static_cast<SpeakerAudioDestination*>(userData);
	speakerAudioDestination->AudioCallback(buffer, length);
}

/*virtual*/ bool SpeakerAudioDestination::GetPlaybackTime(double& playbackTimeSeconds) const
{
	Uint32 sinkRateSampleFramesPerSecond = this->audioSpec.freq;

	Uint32 bitsPerSample = SDL_AUDIO_BITSIZE(this->audioSpec.format);
	Uint32 bytesPerSample = bitsPerSample / 8;

	Uint32 totalSamplesSunk = this->totalBytesSunk / bytesPerSample;
	Uint32 totalSampleFramesSunk = totalSamplesSunk / this->audioSpec.channels;

	double totalSinkTimeSeconds = double(totalSampleFramesSunk) / double(sinkRateSampleFramesPerSecond);
	
	playbackTimeSeconds = this->initialPlaybackTimeSeconds + totalSinkTimeSeconds;
	return true;
}

/*virtual*/ bool SpeakerAudioDestination::SetPlaybackTime(double playbackTimeSeconds)
{
	if (this->deviceID != 0 && this->audioStream)
	{
		SDL_LockAudioDevice(this->deviceID);
		this->initialPlaybackTimeSeconds = playbackTimeSeconds;
		this->totalBytesSunk = 0;
		SDL_AudioStreamClear(this->audioStream);
		SDL_UnlockAudioDevice(this->deviceID);
	}
	else
	{
		this->initialPlaybackTimeSeconds = playbackTimeSeconds;
		this->totalBytesSunk = 0;
	}

	return true;
}

/*virtual*/ std::string SpeakerAudioDestination::GetStatusMessage() const
{
	double playbackTimeSeconds = 0.0;
	this->GetPlaybackTime(playbackTimeSeconds);
	return std::format("Audio playback time: {:.2f}", playbackTimeSeconds);
}