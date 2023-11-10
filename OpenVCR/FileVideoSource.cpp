#include "FileVideoSource.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

FileVideoSource::FileVideoSource(const std::string& givenName) : VideoDevice(givenName)
{
	this->videoFilePath = new std::string();
	this->videoCapture = new cv::VideoCapture();
	this->frame = new cv::Mat();
	this->frameCount = 0;
}

/*virtual*/ FileVideoSource::~FileVideoSource()
{
	delete this->videoFilePath;
	delete this->videoCapture;
	delete this->frame;
}

/*static*/ FileVideoSource* FileVideoSource::Create(const std::string& name)
{
	return new FileVideoSource(name);		// Allocate class in this DLL's heap!
}

void FileVideoSource::SetVideoFilePath(const std::string& videoFilePath)
{
	*this->videoFilePath = videoFilePath;
}

const std::string& FileVideoSource::GetVideoFilePath()
{
	return *this->videoFilePath;
}

/*virtual*/ bool FileVideoSource::PowerOn(Machine* machine, Error& error)
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

/*virtual*/ bool FileVideoSource::PowerOff(Machine* machine, Error& error)
{
	if (this->videoCapture)
	{
		this->videoCapture->release();
		delete this->videoCapture;
		this->videoCapture = nullptr;
	}

	return true;
}

/*virtual*/ bool FileVideoSource::MoveData(Machine* machine, Error& error)
{
	if (!this->videoCapture || !this->videoCapture->isOpened())
	{
		error.Add("Video capture not setup!");
		return false;
	}

	long position = -1;
	switch (machine->GetDisposition(position))
	{
		case Machine::Disposition::PULL:
		{
			if (!this->videoCapture->read(*this->frame))
			{
				error.Add("Read method on video capture device failed.");
				return false;
			}

			this->complete = true;
			break;
		}
		case Machine::Disposition::PLACE:
		{
			if (position < 0 || position >= this->frameCount)
			{
				error.Add("Given frame position out of bounds.");
				return false;
			}

			double propertyValue = double(position);
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

			if (!this->videoCapture->retrieve(*this->frame))
			{
				error.Add("Retrieve call failed.");
				return false;
			}

			this->complete = true;
			break;
		}
	}

	return true;
}

/*virtual*/ bool FileVideoSource::GetFrameSize(cv::Size& frameSize, Error& error)
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

/*virtual*/ bool FileVideoSource::GetFrameRate(double& frameRate, Error& error)
{
	if (!this->videoCapture)
	{
		error.Add("Can't return frame-rate if capture device not setup.");
		return false;
	}

	frameRate = this->videoCapture->get(cv::CAP_PROP_FPS);
	return true;
}

bool FileVideoSource::GetCurrentFrameNumber(long& frameNumber, Error& error)
{
	double propertyValue = 0.0;
	propertyValue = this->videoCapture->get(cv::CAP_PROP_POS_FRAMES);
	if (propertyValue == 0.0)
	{
		error.Add("Could not get CAP_PROP_POS_FRAMES property from video capture device.");
		return false;
	}

	frameNumber = (long)propertyValue;
	return true;
}