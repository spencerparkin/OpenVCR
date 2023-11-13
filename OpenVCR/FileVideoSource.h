#pragma once

#include "VideoDevice.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API FileVideoSource : public VideoDevice
	{
	public:
		FileVideoSource(const std::string& givenName);
		virtual ~FileVideoSource();

		static FileVideoSource* Create(const std::string& name);

		void SetVideoFilePath(const std::string& videoFilePath);
		const std::string& GetVideoFilePath();

		bool GetCurrentFrameNumber(long& frameNumber, Error& error);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetFrameSize(cv::Size& frameSize, Error& error) override;
		virtual bool GetFrameRate(double& frameRate, Error& error) override;

	private:
		cv::VideoCapture* videoCapture;
		std::string* videoFilePath;
		long frameCount;
	};
}