#pragma once

#include "VideoDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API RotationFilter : public VideoDevice
	{
	public:
		RotationFilter(const std::string& givenName);
		virtual ~RotationFilter();

		static RotationFilter* Create(const std::string& name);

		virtual bool MoveData(Machine* machine, Error& error) override;

		void SetRotationAngle(double givenRotationAngleDegrees) { this->rotationAngleDegrees = givenRotationAngleDegrees; }
		double GetRotationAngle() const { return this->rotationAngleDegrees; }

	private:
		double rotationAngleDegrees;
	};
}