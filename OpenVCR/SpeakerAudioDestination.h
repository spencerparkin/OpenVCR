#pragma once

#include "AudioDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API SpeakAudioDestination : public AudioDevice
	{
	public:
		SpeakAudioDestination(const std::string& givenName);
		virtual ~SpeakAudioDestination();

		static SpeakAudioDestination* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual int GetSortKey() const override;

	private:

		void AudioCallback(Uint8* buffer, int length);

		static void SDLCALL AudioCallbackEntry(void* userData, Uint8* buffer, int length);

		SDL_AudioDeviceID deviceID;
		Uint32 feedLocation;
		Uint8* audioBuffer;
		Uint32 audioBufferSize;
	};
}