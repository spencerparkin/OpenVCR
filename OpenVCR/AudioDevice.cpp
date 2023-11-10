#include "AudioDevice.h"

using namespace OpenVCR;

AudioDevice::AudioDevice()
{
}

/*virtual*/ AudioDevice::~AudioDevice()
{
}

/*virtual*/ void* AudioDevice::GetSampleData()
{
	return nullptr;
}