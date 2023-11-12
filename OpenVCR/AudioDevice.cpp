#include "AudioDevice.h"

using namespace OpenVCR;

AudioDevice::AudioDevice(const std::string& givenName) : IODevice(givenName)
{
	::memset(&this->audioSpec, 0, sizeof(this->audioSpec));
}

/*virtual*/ AudioDevice::~AudioDevice()
{
}

/*virtual*/ bool AudioDevice::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	return false;
}

/*virtual*/ SDL_AudioSpec* AudioDevice::GetAudioSpec()
{
	return &this->audioSpec;
}

/*virtual*/ Uint32 AudioDevice::GetPlaybackPosition() const
{
	return 0;
}

/*virtual*/ void AudioDevice::SetPlaybackPosition(Uint32 playbackPosition)
{
}