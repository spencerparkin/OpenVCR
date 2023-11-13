#pragma once

#include "AudioDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API VolumeFilter : public AudioDevice
	{
	public:
		VolumeFilter(const std::string& givenName);
		virtual ~VolumeFilter();

		static VolumeFilter* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetSampleData(std::vector<Uint8>& sampleBuffer) override;

		void SetVolume(double volume);
		double GetVolume() const;

	private:
		double volumeFactor;
		std::vector<Uint8>* filteredSampleBuffer;
	};
}