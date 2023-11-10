#pragma once

#include "IODevice.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API FileVideoDestination : public IODevice
	{
	public:
		FileVideoDestination();
		virtual ~FileVideoDestination();

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, bool& moved, Error& error) override;
		virtual int GetSortKey() const override;

		void SetSourceName(const std::string& givenName) { *this->sourceName = givenName; }
		const std::string& GetSourceName() const { return *this->sourceName; }

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
		std::string* sourceName;
	};
}