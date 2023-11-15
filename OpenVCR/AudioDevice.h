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
		double AudioBufferOffsetToTimeSeconds(Uint32 audioBufferOffset, const SDL_AudioSpec* givenAudioSpec = nullptr) const;
		Uint32 AudioBufferOffsetFromTimeSeconds(double timeSeconds, const SDL_AudioSpec* givenAudioSpec = nullptr) const;

		// This is typically the format of the audio produced by the device.
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