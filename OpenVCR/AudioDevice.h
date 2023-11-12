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
		virtual Uint32 GetPlaybackPosition() const;
		virtual void SetPlaybackPosition(Uint32 playbackPosition);

	protected:
		SDL_AudioSpec audioSpec;
	};
}