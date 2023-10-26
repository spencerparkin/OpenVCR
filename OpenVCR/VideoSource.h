#pragma once

#include "Common.h"

namespace OpenVCR
{
	class Error;
	class Frame;

	// Derivatives of this class would include a web-cam, video file with decoding, IP-camera, etc.
	class OPEN_VCR_API VideoSource
	{
	public:
		VideoSource();
		virtual ~VideoSource();

		virtual bool PowerOn(Error& error);
		virtual bool PowerOff(Error& error);
		virtual bool GetFrameCount(long& frameCount, Error& error);
		virtual bool GetFrame(Frame& frame, long i, Error& error);
		virtual bool GetNextFrame(Frame& frame, Error& error);
		virtual bool IsLiveStream();
	};
}