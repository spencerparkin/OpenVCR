#pragma once

#include "AudioDevice.h"
#include "ByteStream.h"

namespace OpenVCR
{
	class OPEN_VCR_API AdditiveAudioMixer : public AudioDevice
	{
	public:
		AdditiveAudioMixer(const std::string& givenName);
		virtual ~AdditiveAudioMixer();

		static AdditiveAudioMixer* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetSampleData(std::vector<Uint8>& sampleBuffer) override;

	private:
		bool IntegrateSample(const std::vector<Uint8>& audioSample, Error& error);

		std::vector<RawByteStream*>* byteStreamArray;
		std::vector<Uint8>* mixedAudioSample;
	};
}