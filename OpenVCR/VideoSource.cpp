#include "VideoSource.h"

using namespace OpenVCR;

VideoSource::VideoSource()
{
	this->videoCapture = nullptr;
}

/*virtual*/ VideoSource::~VideoSource()
{
}

/*virtual*/ bool VideoSource::PowerOn(Error& error)
{
	return false;
}

/*virtual*/ bool VideoSource::PowerOff(Error& error)
{
	return false;
}

/*virtual*/ bool VideoSource::GetFrameCount(long& frameCount, Error& error)
{
	return false;
}

/*virtual*/ bool VideoSource::GetFrameNumber(long& frameNumber, Error& error)
{
	return false;
}

/*virtual*/ bool VideoSource::GetFrame(Frame& frame, long i, Error& error)
{
	return false;
}

/*virtual*/ bool VideoSource::GetNextFrame(Frame& frame, Error& error)
{
	return false;
}

cv::VideoCapture* VideoSource::GetVideoCapture()
{
	return this->videoCapture;
}