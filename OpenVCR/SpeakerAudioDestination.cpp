#include "SpeakerAudioDestination.h"
#include "Machine.h"

using namespace OpenVCR;

SpeakAudioDestination::SpeakAudioDestination(const std::string& givenName) : AudioDevice(givenName)
{
	this->deviceID = 0;
	this->feedLocation = 0;
	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;
}

/*virtual*/ SpeakAudioDestination::~SpeakAudioDestination()
{
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
		error.Add("Can't power-on speak audio destination without source audio device.");
		return false;
	}

	this->audioBuffer = audioDevice->GetSampleData(this->audioBufferSize);
	if (!this->audioBuffer || this->audioBufferSize == 0)
	{
		error.Add("No audio buffer to use.");
		return false;
	}

	::memcpy(&this->audioSpec, audioDevice->GetAudioSpec(), sizeof(SDL_AudioSpec));

	this->audioSpec.callback = &SpeakAudioDestination::AudioCallbackEntry;
	this->audioSpec.userdata = this;

	int numAudioDevices = SDL_GetNumAudioDevices(0);
	if (numAudioDevices == 0)
	{
		error.Add("No output audio devices found.");
		return false;
	}

	const char* audioDeviceName = nullptr;
	for (int i = 0; i < numAudioDevices; i++)
	{
		audioDeviceName = SDL_GetAudioDeviceName(i, 0);
		if (strstr(audioDeviceName, "Logi") != nullptr)
			break;
	}

	SDL_AudioSpec obtainedSpec;
	this->deviceID = SDL_OpenAudioDevice(audioDeviceName, 0, &this->audioSpec, &obtainedSpec, 0);
	if (this->deviceID == 0)
	{
		error.Add(std::format("Failed to open default audio device: {}", SDL_GetError()));
		return false;
	}

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

	return true;
}

/*virtual*/ bool SpeakAudioDestination::MoveData(Machine* machine, Error& error)
{
	double position = 0.0;
	if (machine->GetDisposition(position) == Machine::Disposition::PLACE)
	{
		// TODO: Correct our feedLocation if we've strayed too much from the position.
	}

	this->complete = true;
	return true;
}

void SpeakAudioDestination::AudioCallback(Uint8* buffer, int length)
{
	for (int i = 0; i < length; i++)
	{
		Uint8 byte = 0;
		int j = this->feedLocation + i;
		if (j < (signed)this->audioBufferSize)
			byte = this->audioBuffer[j];

		buffer[i] = byte;
	}

	this->feedLocation += length;
}

/*static*/ void SDLCALL SpeakAudioDestination::AudioCallbackEntry(void* userData, Uint8* buffer, int length)
{
	auto speakerAudioDestination = static_cast<SpeakAudioDestination*>(userData);
	speakerAudioDestination->AudioCallback(buffer, length);
}