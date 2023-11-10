#include "AudioDevice.h"

using namespace OpenVCR;

AudioDevice::AudioDevice(const std::string& givenName) : IODevice(givenName)
{
}

/*virtual*/ AudioDevice::~AudioDevice()
{
}

/*virtual*/ void* AudioDevice::GetSampleData()
{
	return nullptr;
}