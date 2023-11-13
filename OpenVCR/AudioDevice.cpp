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

/*virtual*/ bool AudioDevice::GetPlaybackTime(double& playbackTimeSeconds) const
{
	return false;
}

/*virtual*/ bool AudioDevice::SetPlaybackTime(double playbackTimeSeconds)
{
	return false;
}