#pragma once

#include "IODevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API VideoDevice : public IODevice
	{
	public:
		VideoDevice();
		virtual ~VideoDevice();

		virtual cv::Mat* GetFrameData();
		virtual bool GetFrameSize(cv::Size& frameSize, Error& error);
		virtual bool GetFrameRate(double& frameRate, Error& error);

	protected:
		cv::Mat* frame;
	};
}