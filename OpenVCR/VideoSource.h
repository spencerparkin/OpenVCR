#pragma once

#include "Common.h"

namespace OpenVCR
{
	class Frame;

	// Derivatives of this class would include a web-cam, video file with decoding, IP-camera, etc.
	class OPEN_VCR_API VideoSource
	{
	public:
		VideoSource();
		virtual ~VideoSource();

		virtual bool PowerOn();
		virtual bool PowerOff();
		virtual bool GetFrameCount(int& frameCount);
		virtual bool GetFrame(Frame& frame, int i);
		virtual bool GetNextFrame(Frame& frame);
	};
}