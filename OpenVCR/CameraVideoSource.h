#pragma once

#include "VideoDevice.h"
#include <opencv2/videoio.hpp>

namespace OpenVCR
{
	class OPEN_VCR_API CameraVideoSource : public VideoDevice
	{
	public:
		CameraVideoSource();
		virtual ~CameraVideoSource();

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetFrameSize(cv::Size& frameSize, Error& error) override;
		virtual bool GetFrameRate(double& frameRate, Error& error) override;

		// Note that web-cams can be configured for IP streaming, so you don't need an IP camera, per se.
		void SetCameraURL(const std::string& cameraURL);
		void SetCameraURL(const std::string& username, const std::string& password, const std::string& ipAddress, int port);
		const std::string& GetCameraURL() const;

		void SetDeviceNumber(int deviceNumber);
		int GetDeviceNumber() const;

		long GetFrameNumber();

	private:
		cv::VideoCapture* videoCapture;
		std::string* cameraURL;
		int deviceNumber;
		long frameNumber;
	};
}