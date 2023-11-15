#pragma once

#include "AudioDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API NetworkAudioSource : public AudioDevice
	{
	public:
		NetworkAudioSource(const std::string& givenName);
		virtual ~NetworkAudioSource();

		static NetworkAudioSource* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetSampleData(std::vector<Uint8>& sampleBuffer) override;

	private:
	};
}