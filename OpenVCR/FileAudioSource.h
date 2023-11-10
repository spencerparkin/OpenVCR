#pragma once

#include "AudioDevice.h"
#include <SDL.h>

namespace OpenVCR
{
	class OPEN_VCR_API FileAudioSource : public AudioDevice
	{
	public:
		FileAudioSource(const std::string& givenName);
		virtual ~FileAudioSource();

		static FileAudioSource* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual Uint8* GetSampleData(Uint32& size) override;

		void SetAudioFilePath(const std::string& audioFilePath);
		const std::string& GetAudioFilePath() const;

	private:
		std::string* audioFilePath;
		Uint8* audioBuffer;
		Uint32 audioBufferSize;
	};
}