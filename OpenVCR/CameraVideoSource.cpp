#include "CameraVideoSource.h"
#include "Error.h"
#include "Frame.h"

using namespace OpenVCR;

CameraVideoSource::CameraVideoSource()
{
	this->videoCapture = nullptr;
	this->deviceNumber = 0;
	this->cameraURL = new std::string();
	this->frameNumber = 0;
}

/*virtual*/ CameraVideoSource::~CameraVideoSource()
{
	delete this->cameraURL;
}

/*virtual*/ bool CameraVideoSource::PowerOn(Error& error)
{
	if (this->videoCapture)
	{
		error.Add("Capture device already created.");
		return false;
	}

	this->videoCapture = new cv::VideoCapture();

	if (this->cameraURL->size() > 0)
		this->videoCapture->open(*this->cameraURL);
	else
		this->videoCapture->open(this->deviceNumber);

	if (!this->videoCapture->isOpened())
	{
		error.Add("Video capture device failed to open.");		// TODO: Can we get any better error information?
		return false;
	}

	return true;
}

/*virtual*/ bool CameraVideoSource::PowerOff(Error& error)
{
	if (this->videoCapture)
	{
		this->videoCapture->release();
		delete this->videoCapture;
		this->videoCapture = nullptr;
	}

	return true;
}

/*virtual*/ bool CameraVideoSource::GetFrameCount(long& frameCount, Error& error)
{
	error.Add("No frame-count for a live feed.");
	return false;
}

/*virtual*/ bool CameraVideoSource::GetFrameNumber(long& frameNumber, Error& error)
{
	frameNumber = this->frameNumber;
	return true;
}

/*virtual*/ bool CameraVideoSource::GetFrame(Frame& frame, long i, Error& error)
{
	error.Add("Can't get specific frame for a live feed.");
	return false;
}

/*virtual*/ bool CameraVideoSource::GetNextFrame(Frame& frame, Error& error)
{
	if (!this->videoCapture)
	{
		error.Add("Video capture device not yet created.");
		return false;
	}

	if (!this->videoCapture->read(*frame.data))
	{
		error.Add("Failed to read from video capture device.");
		return false;
	}

	this->frameNumber++;
	return true;
}

void CameraVideoSource::SetCameraURL(const std::string& cameraURL)
{
	*this->cameraURL = cameraURL;
}

void CameraVideoSource::SetCameraURL(const std::string& username, const std::string& password, const std::string& ipAddress, int port)
{
	char portStr[64];
	sprintf_s(portStr, sizeof(portStr), "%d", port);
	*this->cameraURL = std::format("rtsp://{}:{}@{}:{}", username.c_str(), password.c_str(), ipAddress.c_str(), portStr);
}

const std::string& CameraVideoSource::GetCameraURL() const
{
	return *this->cameraURL;
}

void CameraVideoSource::SetDeviceNumber(int deviceNumber)
{
	this->deviceNumber = deviceNumber;
}

int CameraVideoSource::GetDeviceNumber() const
{
	return this->deviceNumber;
}