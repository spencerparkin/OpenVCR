#pragma once

#include "VideoDestination.h"

namespace OpenVCR
{
	class OPEN_VCR_API WindowVideoDestination : public VideoDestination
	{
	public:
		WindowVideoDestination();
		virtual ~WindowVideoDestination();

		void SetWindowHandle(HANDLE windowHandle);
		HANDLE GetWindowHandle();

		virtual bool AddFrame(Frame& frame) override;

	private:
		HANDLE windowHandle;
	};
}