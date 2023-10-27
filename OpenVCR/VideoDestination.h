#pragma once

#include "Common.h"

namespace OpenVCR
{
	class Error;
	class Frame;
	class Machine;

	// Derivatives of this class do stuff with frames generated from a video source, such
	// as dump to file or screen using various encodings or graphics APIS (e.g., DirectX or OpenGL.)
	class OPEN_VCR_API VideoDestination
	{
	public:
		VideoDestination();
		virtual ~VideoDestination();

		virtual bool PowerOn(Machine* machine, Error& error);
		virtual bool PowerOff(Error& error);
		virtual bool AddFrame(Frame& frame, Error& error);
	};
}