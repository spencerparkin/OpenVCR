#include "AudioDevice.h"
#include "Error.h"

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

bool AudioDevice::SelectAudioDevice(std::string& chosenDevice, const std::string& deviceSubStr, DeviceType deviceType, Error& error)
{
	return this->SelectAudioDevice(chosenDevice, [=](const std::string& deviceName) -> bool {
		return deviceName.find(deviceSubStr) >= 0;
	}, deviceType, error);
}

bool AudioDevice::SelectAudioDevice(std::string& chosenDevice, DeviceSelectionCallback selectionCallback, DeviceType deviceType, Error& error)
{
	int isCapture = (deviceType == DeviceType::INPUT) ? 1 : 0;
	int numAudioDevices = SDL_GetNumAudioDevices(isCapture);
	if (numAudioDevices == 0)
	{
		error.Add("No input audio devices found.");
		return false;
	}

	int i;
	for (i = 0; i < numAudioDevices; i++)
	{
		std::string audioDeviceName = SDL_GetAudioDeviceName(i, isCapture);
		if (selectionCallback(audioDeviceName))
		{
			chosenDevice = audioDeviceName;
			return true;
		}
	}

	if (i == numAudioDevices)
	{
		error.Add("You didn't choose any of the available devices.");
		return false;
	}

	return true;
}