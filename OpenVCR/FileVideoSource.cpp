#include "FileVideoSource.h"
#include "Error.h"
#include "Frame.h"

using namespace OpenVCR;

FileVideoSource::FileVideoSource()
{
	this->videoFilePath = new std::string();
	this->videoCapture = nullptr;
	this->frameCount = 0;
}

/*virtual*/ FileVideoSource::~FileVideoSource()
{
	delete this->videoFilePath;
	delete this->videoCapture;
}

void FileVideoSource::SetVideoFilePath(const std::string& videoFilePath)
{
	*this->videoFilePath = videoFilePath;
}

const std::string& FileVideoSource::GetVideoFilePath()
{
	return *this->videoFilePath;
}

/*virtual*/ bool FileVideoSource::PowerOn(Error& error)
{
	// TODO: We should be more configurable here so that the user can indicate which decoding to use, etc.
	this->videoCapture = new cv::VideoCapture(*this->videoFilePath, cv::VideoCaptureAPIs::CAP_FFMPEG);
	if (!this->videoCapture->isOpened())
	{
		error.Add("Failed to open video: " + *this->videoFilePath);
		return false;
	}

	double propertyValue = this->videoCapture->get(cv::CAP_PROP_FRAME_COUNT);
	if (propertyValue == 0.0)
	{
		error.Add("cv::CAP_PROP_FRAME_COUNT property not supported by video capture backend.");
		return false;
	}
	this->frameCount = long(propertyValue);

	return true;
}

/*virtual*/ bool FileVideoSource::PowerOff(Error& error)
{
	delete this->videoCapture;
	this->videoCapture = nullptr;

	return true;
}

/*virtual*/ bool FileVideoSource::GetFrameCount(long& frameCount, Error& error)
{
	frameCount = this->frameCount;
	return true;
}

/*virtual*/ bool FileVideoSource::GetFrame(Frame& frame, long i, Error& error)
{
	if (!this->videoCapture || !this->videoCapture->isOpened())
	{
		error.Add("Can't get frame if video capture is not open.");
		return false;
	}

	double propertyValue = double(i);		// TODO: Stack overflow says we need to use i-1.  Confirm?
	if (!this->videoCapture->set(cv::CAP_PROP_POS_FRAMES, propertyValue))
	{
		error.Add("Video capture backend does not support setting the frame position.");
		return false;
	}

	if (!this->videoCapture->grab())
	{
		error.Add("Grab failed.  Maybe device is at end of video?");
		return false;
	}

	if (!this->videoCapture->retrieve(*frame.data))
	{
		error.Add("Retrieve call failed.");
		return false;
	}

	return true;
}

/*virtual*/ bool FileVideoSource::GetNextFrame(Frame& frame, Error& error)
{
	error.Add("A file video source is not a live-stream.");
	return false;
}

/*virtual*/ bool FileVideoSource::IsLiveStream()
{
	return false;
}