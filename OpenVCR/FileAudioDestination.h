#pragma once

#include "AudioDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API FileAudioDestination : public AudioDevice
	{
	public:
		FileAudioDestination(const std::string& givenName);
		virtual ~FileAudioDestination();

		static FileAudioDestination* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;

		void SetAudioFilePath(const std::string& audioFilePath);
		const std::string& GetAudioFilePath() const;

	private:

		SDL_AudioSpec inputSpec;
		SDL_AudioSpec outputSpec;
		SDL_AudioStream* audioStream;
		std::string* audioFilePath;
	};
}