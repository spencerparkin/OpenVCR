#pragma once

#include "IODevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API RotationFilter : public IODevice
	{
	public:
		RotationFilter();
		virtual ~RotationFilter();

		virtual bool PreTick(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, bool& moved, Error& error) override;
		virtual cv::Mat* GetFrameData() override;
		
		void SetSourceName(const std::string& givenName) { *this->sourceName = givenName; }
		const std::string& GetSourceName() const { return *this->sourceName; }

		void SetRotationAngle(double givenRotationAngleDegrees) { this->rotationAngleDegrees = givenRotationAngleDegrees; }
		double GetRotationAngle() const { return this->rotationAngleDegrees; }

	private:
		std::string* sourceName;
		double rotationAngleDegrees;
		cv::Mat* frame;
		bool frameReady;
	};
}