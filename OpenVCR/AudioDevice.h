#pragma once

#include "IODevice.h"
#include <SDL.h>

namespace OpenVCR
{
	class OPEN_VCR_API AudioDevice : public IODevice
	{
	public:
		AudioDevice(const std::string& givenName);
		virtual ~AudioDevice();

		virtual Uint8* GetSampleData(Uint32& size);
		virtual SDL_AudioSpec* GetAudioSpec();

	protected:
		SDL_AudioSpec audioSpec;
	};
}