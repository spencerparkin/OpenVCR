#pragma once

#include "Common.h"

namespace OpenVCR
{
	class Error;
	class Frame;

	// Derivatives of this class would include a window, DirectX texture, a video file with encoding, etc.
	class OPEN_VCR_API VideoDestination
	{
	public:
		VideoDestination();
		virtual ~VideoDestination();

		virtual bool PowerOn(Error& error);
		virtual bool PowerOff(Error& error);
		virtual bool AddFrame(Frame& frame, Error& error);
	};
}