#pragma once

#include "AudioDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API SpeakerAudioDestination : public AudioDevice
	{
	public:
		SpeakerAudioDestination(const std::string& givenName);
		virtual ~SpeakerAudioDestination();

		static SpeakerAudioDestination* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetPlaybackTime(double& playbackTimeSeconds) const override;
		virtual bool SetPlaybackTime(double playbackTimeSeconds) override;
		virtual std::string GetStatusMessage() const override;

		void SetDeviceSubString(const std::string& deviceSubStr);

	private:

		void AudioCallback(Uint8* buffer, int length);

		static void SDLCALL AudioCallbackEntry(void* userData, Uint8* buffer, int length);

		std::string* deviceSubStr;
		SDL_AudioDeviceID deviceID;
		std::vector<Uint8>* machineThreadBuffer;
		SDL_AudioStream* audioStream;
		double initialPlaybackTimeSeconds;
		Uint32 totalBytesSunk;
	};
}