#include "CameraVideoSource.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

CameraVideoSource::CameraVideoSource(const std::string& givenName) : VideoDevice(givenName)
{
	this->deviceNumber = 0;
	this->cameraURL = new std::string();
	this->videoCapture = nullptr;
	this->frameNumber = 0;
}

/*virtual*/ CameraVideoSource::~CameraVideoSource()
{
	delete this->cameraURL;
	delete this->videoCapture;
}

/*static*/ CameraVideoSource* CameraVideoSource::Create(const std::string& name)
{
	return new CameraVideoSource(name);		// Allocate class in this DLL's heap!
}

/*virtual*/ bool CameraVideoSource::PowerOn(Machine* machine, Error& error)
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

	this->frameNumber = 0;
	return true;
}

/*virtual*/ bool CameraVideoSource::PowerOff(Machine* machine, Error& error)
{
	if (this->videoCapture)
	{
		this->videoCapture->release();
		delete this->videoCapture;
		this->videoCapture = nullptr;
	}

	this->frameNumber = 0;
	return true;
}

/*virtual*/ bool CameraVideoSource::MoveData(Machine* machine, Error& error)
{
	if (!this->videoCapture || !this->videoCapture->isOpened())
	{
		error.Add("Video capture device not yet setup.");
		return false;
	}

	long position = -1;
	switch (machine->GetDisposition(position))
	{
		case Machine::Disposition::PULL:
		{
			if (!this->videoCapture->read(*this->frame))
			{
				error.Add("Failed to read from video capture device.");
				return false;
			}

			this->frameNumber++;
			this->complete = true;
			break;
		}
		case Machine::Disposition::PLACE:
		{
			error.Add("Can't place a live feed at a given position.");
			return false;
		}
	}

	return true;
}

/*virtual*/ bool CameraVideoSource::GetFrameSize(cv::Size& frameSize, Error& error)
{
	if (!this->videoCapture)
	{
		error.Add("Can't return size if capture device not setup.");
		return false;
	}

	frameSize.width = (int)videoCapture->get(cv::CAP_PROP_FRAME_WIDTH);
	frameSize.height = (int)videoCapture->get(cv::CAP_PROP_FRAME_HEIGHT);

	return true;
}

/*virtual*/ bool CameraVideoSource::GetFrameRate(double& frameRate, Error& error)
{
	if (!this->videoCapture)
	{
		error.Add("Can't return frame-rate if capture device not setup.");
		return false;
	}

	frameRate = this->videoCapture->get(cv::CAP_PROP_FPS);
	return true;
}

long CameraVideoSource::GetFrameNumber()
{
	return this->frameNumber;
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