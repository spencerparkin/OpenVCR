#pragma once

#include "VideoDestination.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API FileVideoDestination : public VideoDestination
	{
	public:
		FileVideoDestination();
		virtual ~FileVideoDestination();

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Error& error) override;
		virtual bool AddFrame(Frame& frame, Error& error) override;

		void SetVideoFilePath(const std::string& videoFilePath);
		const std::string& GetVideoFilePath() const;

		void Pause();
		void Resume();
		bool IsPaused() const;

	private:
		cv::VideoWriter* videoWriter;
		std::string* videoFilePath;
		double frameRateFPS;
		int encoderFourCC;
		cv::Size* frameSize;
		bool suspendFrameWrites;
	};
}