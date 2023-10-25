#pragma once

#include "Common.h"

namespace OpenVCR
{
	class Frame;

	// Derivatives of this class would include a window, DirectX texture, a video file with encoding, etc.
	class OPEN_VCR_API VideoDestination
	{
	public:
		VideoDestination();
		virtual ~VideoDestination();

		bool AddFrame(Frame& frame);
	};
}