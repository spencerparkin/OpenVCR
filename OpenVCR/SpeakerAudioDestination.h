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