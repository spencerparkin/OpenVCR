#pragma once

#include "VideoSource.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API FileVideoSource : public VideoSource
	{
	public:
		FileVideoSource();
		virtual ~FileVideoSource();

		void SetVideoFilePath(const std::string& videoFilePath);
		const std::string& GetVideoFilePath();

		virtual bool PowerOn(Error& error) override;
		virtual bool PowerOff(Error& error) override;
		virtual bool GetFrameCount(long& frameCount, Error& error) override;
		virtual bool GetFrame(Frame& frame, long i, Error& error) override;
		virtual bool GetNextFrame(Frame& frame, Error& error) override;
		virtual bool IsLiveStream() override;

	private:

		std::string* videoFilePath;
		cv::VideoCapture* videoCapture;
		long frameCount;
	};
}