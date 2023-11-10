#include "AudioDevice.h"

using namespace OpenVCR;

AudioDevice::AudioDevice(const std::string& givenName) : IODevice(givenName)
{
	::memset(&this->audioSpec, 0, sizeof(this->audioSpec));
}

/*virtual*/ AudioDevice::~AudioDevice()
{
}

/*virtual*/ Uint8* AudioDevice::GetSampleData(Uint32& size)
{
	size = 0;
	return nullptr;
}

/*virtual*/ SDL_AudioSpec* AudioDevice::GetAudioSpec()
{
	return &this->audioSpec;
}