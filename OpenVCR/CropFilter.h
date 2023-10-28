#pragma once

#include "FrameFilter.h"

namespace OpenVCR
{
	class OPEN_VCR_API CropFilter : public FrameFilter
	{
	public:
		CropFilter();
		virtual ~CropFilter();

		virtual bool Filter(Frame& inputFrame, Frame& outputFrame, Error& error) override;

		int leftCrop;
		int rightCrop;
		int topCrop;
		int bottomCrop;
	};
}