#pragma once

#include "VideoSource.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API CameraVideoSource : public VideoSource
	{
	public:
		CameraVideoSource();
		virtual ~CameraVideoSource();

		virtual bool PowerOn(Error& error) override;
		virtual bool PowerOff(Error& error) override;
		virtual bool GetFrameCount(long& frameCount, Error& error) override;
		virtual bool GetFrameNumber(long& frameNumber, Error& error) override;
		virtual bool GetFrame(Frame& frame, long i, Error& error) override;
		virtual bool GetNextFrame(Frame& frame, Error& error) override;

		// Note that web-cams can be configured for IP streaming, so you don't need an IP camera, per se.
		void SetCameraURL(const std::string& cameraURL);
		void SetCameraURL(const std::string& username, const std::string& password, const std::string& ipAddress, int port);
		const std::string& GetCameraURL() const;

		void SetDeviceNumber(int deviceNumber);
		int GetDeviceNumber() const;

	private:

		cv::VideoCapture* videoCapture;
		std::string* cameraURL;
		int deviceNumber;
		long frameNumber;
	};
}