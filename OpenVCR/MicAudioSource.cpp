#include "MicAudioSource.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

MicAudioSource::MicAudioSource(const std::string& givenName) : AudioDevice(givenName)
{
	this->deviceID = 0;
	this->deviceSubStr = new std::string();
	this->audioThreadBuffer = new std::vector<Uint8>();
	this->machineThreadBuffer = new std::vector<Uint8>();
}

/*virtual*/ MicAudioSource::~MicAudioSource()
{
	delete this->deviceSubStr;
	delete this->audioThreadBuffer;
	delete this->machineThreadBuffer;
}

/*static*/ MicAudioSource* MicAudioSource::Create(const std::string& name)
{
	return new MicAudioSource(name);
}

/*virtual*/ bool MicAudioSource::PowerOn(Machine* machine, Error& error)
{
	if (this->deviceID != 0)
	{
		error.Add("Audio device ID already non-zero!");
		return false;
	}

	this->machineThreadBuffer->clear();
	this->audioThreadBuffer->clear();

	int numAudioDevices = SDL_GetNumAudioDevices(1);
	if (numAudioDevices == 0)
	{
		error.Add("No input audio devices found.");
		return false;
	}

	const char* audioDeviceName = nullptr;
	if (this->deviceSubStr->length() > 0)
	{
		for (int i = 0; i < numAudioDevices; i++)
		{
			audioDeviceName = SDL_GetAudioDeviceName(i, 1);
			if (strstr(audioDeviceName, this->deviceSubStr->c_str()) != nullptr)
				break;
		}
	}

	SDL_AudioSpec desiredSpec;
	desiredSpec.size = 0;
	desiredSpec.padding = 0;
	desiredSpec.samples = 4096;
	desiredSpec.silence = 0;
	desiredSpec.channels = 1;
	desiredSpec.format = AUDIO_S16LSB;
	desiredSpec.freq = 22050;
	desiredSpec.callback = &MicAudioSource::AudioCallbackEntry;
	desiredSpec.userdata = this;

	this->deviceID = SDL_OpenAudioDevice(audioDeviceName, 1, &desiredSpec, &this->audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (this->deviceID == 0)
	{
		error.Add(std::format("Failed to open audio device: {}", SDL_GetError()));
		return false;
	}

	SDL_PauseAudioDevice(this->deviceID, 0);
	this->poweredOn = true;
	return true;
}

/*virtual*/ bool MicAudioSource::PowerOff(Machine* machine, Error& error)
{
	if (this->deviceID != 0)
	{
		SDL_CloseAudioDevice(this->deviceID);
		this->deviceID = 0;
	}

	this->machineThreadBuffer->clear();
	this->audioThreadBuffer->clear();

	this->poweredOn = false;
	return true;
}

/*virtual*/ bool MicAudioSource::MoveData(Machine* machine, Error& error)
{
	SDL_LockAudioDevice(this->deviceID);
	for (Uint8 byte : *this->audioThreadBuffer)
		this->machineThreadBuffer->push_back(byte);
	this->audioThreadBuffer->clear();
	SDL_UnlockAudioDevice(this->deviceID);

	this->complete = true;
	return true;
}

/*virtual*/ bool MicAudioSource::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	sampleBuffer.clear();
	for (Uint8 byte : *this->machineThreadBuffer)
		sampleBuffer.push_back(byte);
	this->machineThreadBuffer->clear();
	return sampleBuffer.size() > 0;
}

void MicAudioSource::SetDeviceSubString(const std::string& deviceSubStr)
{
	*this->deviceSubStr = deviceSubStr;
}

void MicAudioSource::AudioCallback(Uint8* buffer, int length)
{
	for (int i = 0; i < length; i++)
		this->audioThreadBuffer->push_back(buffer[i]);
}

/*static*/ void SDLCALL MicAudioSource::AudioCallbackEntry(void* userData, Uint8* buffer, int length)
{
	auto micAudioSource = static_cast<MicAudioSource*>(userData);
	micAudioSource->AudioCallback(buffer, length);
}