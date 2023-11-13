#pragma once

#include "IODevice.h"
#include <SDL.h>
#include <vector>

namespace OpenVCR
{
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
	};
}