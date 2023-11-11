#pragma once

#include "AudioDevice.h"
#include <SDL.h>

namespace OpenVCR
{
	class OPEN_VCR_API MicAudioSource : public AudioDevice
	{
	public:
		MicAudioSource(const std::string& givenName);
		virtual ~MicAudioSource();

		static MicAudioSource* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetSampleData(std::vector<Uint8>& sampleBuffer) override;
		virtual int GetSortKey() const override;

		void SetDeviceSubString(const std::string& deviceSubStr);

	private:

		void AudioCallback(Uint8* buffer, int length);

		static void SDLCALL AudioCallbackEntry(void* userData, Uint8* buffer, int length);

		std::string* deviceSubStr;
		SDL_AudioDeviceID deviceID;
		std::vector<Uint8>* audioThreadBuffer;
		std::vector<Uint8>* machineThreadBuffer;
	};
}