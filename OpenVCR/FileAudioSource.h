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
		virtual bool GetSampleData(std::vector<Uint8>& sampleBuffer) override;

		void SetAudioFilePath(const std::string& audioFilePath);
		const std::string& GetAudioFilePath() const;

		void SetAudioSinkName(const std::string& audioSinkName);
		const std::string& GetAudioSinkName() const;

	private:
		std::string* audioSinkName;
		std::string* audioFilePath;
		Uint8* audioBuffer;
		Uint32 audioBufferSize;
		Uint32 playbackPosition;
		Uint32 playbackChunkSizeBytes;
		Uint32 nextSampleStart;
		Uint32 nextSampleEnd;
	};
}