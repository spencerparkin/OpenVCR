#pragma once

#include "Common.h"

namespace OpenVCR
{
	class Frame;
	class Error;

	class OPEN_VCR_API FrameFilter
	{
	public:
		FrameFilter();
		virtual ~FrameFilter();

		virtual bool Filter(Frame& inputFrame, Frame& outputFrame, Error& error);

		std::string* name;
	};
}