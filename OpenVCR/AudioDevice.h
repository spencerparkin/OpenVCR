#pragma once

#include "IODevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API AudioDevice : public IODevice
	{
	public:
		AudioDevice();
		virtual ~AudioDevice();

		virtual void* GetSampleData();

	protected:
	};
}