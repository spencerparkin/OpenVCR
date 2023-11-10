#pragma once

#include "VideoDevice.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API FileVideoDestination : public VideoDevice
	{
	public:
		FileVideoDestination(const std::string& givenName);
		virtual ~FileVideoDestination();

		static FileVideoDestination* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual int GetSortKey() const override;

		void SetVideoFilePath(const std::string& videoFilePath);
		const std::string& GetVideoFilePath() const;

		void SetFrameSize(int width, int height);
		void SetFrameRate(double frameRateFPS);

		void Pause();
		void Resume();
		bool IsPaused() const;

	private:
		cv::VideoWriter* videoWriter;
		std::string* videoFilePath;
		bool suspendFrameWrites;
		cv::Size* frameSize;
		double frameRateFPS;
	};
}