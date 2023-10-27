#pragma once

#include "Common.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class Error;
	class Frame;

	// This abstraction might be silly since the vc::videoCapture class already is this type of abstraction.
	// In any case, this is OpenVCR's abstraction layer for vc::videoCapture in the context of the derivative.
	class OPEN_VCR_API VideoSource
	{
	public:
		VideoSource();
		virtual ~VideoSource();

		virtual bool PowerOn(Error& error);
		virtual bool PowerOff(Error& error);
		virtual bool GetFrameCount(long& frameCount, Error& error);
		virtual bool GetFrameNumber(long& frameNumber, Error& error);
		virtual bool GetFrame(Frame& frame, long i, Error& error);
		virtual bool GetNextFrame(Frame& frame, Error& error);

		cv::VideoCapture* GetVideoCapture();

	protected:
		cv::VideoCapture* videoCapture;
	};
}