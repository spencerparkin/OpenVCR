#include "VideoDevice.h"

using namespace OpenVCR;

VideoDevice::VideoDevice(const std::string& givenName) : IODevice(givenName)
{
	this->frame = new cv::Mat();
}

/*virtual*/ VideoDevice::~VideoDevice()
{
	delete this->frame;
}

/*virtual*/ cv::Mat* VideoDevice::GetFrameData()
{
	return this->frame;
}

/*virtual*/ bool VideoDevice::GetFrameSize(cv::Size& frameSize, Error& error)
{
	frameSize = this->frame->size();
	return true;
}

/*virtual*/ bool VideoDevice::GetFrameRate(double& frameRate, Error& error)
{
	frameRate = 30.0;
	return true;
}