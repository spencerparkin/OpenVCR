#pragma once

#include "IODevice.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API FileVideoSource : public IODevice
	{
	public:
		FileVideoSource();
		virtual ~FileVideoSource();

		void SetVideoFilePath(const std::string& videoFilePath);
		const std::string& GetVideoFilePath();

		bool GetCurrentFrameNumber(long& frameNumber, Error& error);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool PreTick(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, bool& moved, Error& error) override;
		virtual cv::Mat* GetFrameData() override;
		virtual bool GetFrameSize(cv::Size& frameSize, Error& error) override;
		virtual bool GetFrameRate(double& frameRate, Error& error) override;

	private:

		bool frameReady;
		cv::Mat* frame;
		cv::VideoCapture* videoCapture;
		std::string* videoFilePath;
		long frameCount;
	};
}