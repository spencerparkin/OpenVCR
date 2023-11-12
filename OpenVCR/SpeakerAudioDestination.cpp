#include "SpeakerAudioDestination.h"
#include "Machine.h"

using namespace OpenVCR;

SpeakAudioDestination::SpeakAudioDestination(const std::string& givenName) : AudioDevice(givenName)
{
	this->deviceID = 0;
	this->deviceSubStr = new std::string();
	this->machineThreadBuffer = new std::vector<Uint8>();
	this->audioStream = nullptr;
	this->playbackPosition = 0;
}

/*virtual*/ SpeakAudioDestination::~SpeakAudioDestination()
{
	delete this->deviceSubStr;
	delete this->machineThreadBuffer;
}

/*static*/ SpeakAudioDestination* SpeakAudioDestination::Create(const std::string& name)
{
	return new SpeakAudioDestination(name);
}

/*virtual*/ int SpeakAudioDestination::GetSortKey() const
{
	return 1;
}

/*virtual*/ bool SpeakAudioDestination::PowerOn(Machine* machine, Error& error)
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
	::memcpy(&this->audioSpec, sourceSpec, sizeof(SDL_AudioSpec));
	this->audioSpec.callback = &SpeakAudioDestination::AudioCallbackEntry;
	this->audioSpec.userdata = this;

	int numAudioDevices = SDL_GetNumAudioDevices(0);
	if (numAudioDevices == 0)
	{
		error.Add("No output audio devices found.");
		return false;
	}

	const char* audioDeviceName = nullptr;
	if (this->deviceSubStr->length() > 0)
	{
		for (int i = 0; i < numAudioDevices; i++)
		{
			audioDeviceName = SDL_GetAudioDeviceName(i, 0);
			if (strstr(audioDeviceName, this->deviceSubStr->c_str()) != nullptr)
				break;
		}
	}

	this->deviceID = SDL_OpenAudioDevice(audioDeviceName, 0, sourceSpec, &this->audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
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

	this->playbackPosition = 0;
	SDL_PauseAudioDevice(this->deviceID, 0);
	return true;
}

/*virtual*/ bool SpeakAudioDestination::PowerOff(Machine* machine, Error& error)
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

	return true;
}

void SpeakAudioDestination::SetDeviceSubString(const std::string& deviceSubStr)
{
	*this->deviceSubStr = deviceSubStr;
}

/*virtual*/ bool SpeakAudioDestination::MoveData(Machine* machine, Error& error)
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

	SDL_LockAudioDevice(this->deviceID);
	SDL_AudioStreamPut(this->audioStream, this->machineThreadBuffer->data(), (int)this->machineThreadBuffer->size());
	SDL_UnlockAudioDevice(this->deviceID);

	this->complete = true;
	return true;
}

void SpeakAudioDestination::AudioCallback(Uint8* buffer, int length)
{
	int numBytesGotten = SDL_AudioStreamGet(this->audioStream, buffer, length);
	for (int i = numBytesGotten; i < length; i++)
		buffer[i] = this->audioSpec.silence;

	this->playbackPosition += numBytesGotten;
}

/*static*/ void SDLCALL SpeakAudioDestination::AudioCallbackEntry(void* userData, Uint8* buffer, int length)
{
	auto speakerAudioDestination = static_cast<SpeakAudioDestination*>(userData);
	speakerAudioDestination->AudioCallback(buffer, length);
}

/*virtual*/ Uint32 SpeakAudioDestination::GetPlaybackPosition() const
{
	return this->playbackPosition;
}

/*virtual*/ void SpeakAudioDestination::SetPlaybackPosition(Uint32 playbackPosition)
{
	SDL_LockAudioDevice(this->deviceID);
	this->playbackPosition = playbackPosition;
	SDL_AudioStreamClear(this->audioStream);
	SDL_UnlockAudioDevice(this->deviceID);
}