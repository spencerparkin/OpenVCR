#pragma once

#include "IODevice.h"
#include <SDL.h>
#include <vector>

namespace OpenVCR
{
	class Error;

	class OPEN_VCR_API AudioDevice : public IODevice
	{
	public:
		AudioDevice(const std::string& givenName);
		virtual ~AudioDevice();

		virtual bool GetSampleData(std::vector<Uint8>& sampleBuffer);
		virtual SDL_AudioSpec* GetAudioSpec();
		virtual bool GetPlaybackTime(double& playbackTimeSeconds) const;
		virtual bool SetPlaybackTime(double playbackTimeSeconds);

	protected:

		SDL_AudioSpec audioSpec;

		typedef std::function<bool(const std::string&)> DeviceSelectionCallback;

		enum class DeviceType
		{
			INPUT,
			OUTPUT
		};

		bool SelectAudioDevice(std::string& chosenDevice, const std::string& deviceSubStr, DeviceType deviceType, Error& error);
		bool SelectAudioDevice(std::string& chosenDevice, DeviceSelectionCallback selectionCallback, DeviceType deviceType, Error& error);
	};
}