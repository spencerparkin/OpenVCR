#pragma once

#include "FrameFilter.h"

namespace OpenVCR
{
	class OPEN_VCR_API RotationFilter : public FrameFilter
	{
	public:
		RotationFilter();
		virtual ~RotationFilter();

		virtual bool Filter(Frame& inputFrame, Frame& outputFrame, Error& error) override;

		double rotationAngleDegrees;
	};
}