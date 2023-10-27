#include "FileVideoDestination.h"
#include "Error.h"
#include "Machine.h"
#include "VideoSource.h"

using namespace OpenVCR;

FileVideoDestination::FileVideoDestination()
{
	this->videoWriter = nullptr;
	this->videoFilePath = new std::string();
	this->frameRateFPS = 30.0;
	this->encoderFourCC = 0;
	this->frameSize = new cv::Size(0, 0);
}

/*virtual*/ FileVideoDestination::~FileVideoDestination()
{
	delete this->videoFilePath;
	delete this->frameSize;
}

/*virtual*/ bool FileVideoDestination::PowerOn(Machine* machine, Error& error)
{
	if (this->videoWriter)
	{
		error.Add("Video writer already created.");
		return false;
	}

	cv::VideoCapture* videoCapture = nullptr;
	VideoSource* videoSource = machine->GetVideoSource();
	if (videoSource)
		videoCapture = videoSource->GetVideoCapture();

	if (this->frameSize->width == 0 && this->frameSize->height == 0 && videoCapture)
	{
		this->frameSize->width = (int)videoCapture->get(cv::CAP_PROP_FRAME_WIDTH);
		this->frameSize->height = (int)videoCapture->get(cv::CAP_PROP_FRAME_HEIGHT);
	}

	if (this->frameSize->width == 0 && this->frameSize->height == 0)
	{
		error.Add("Could not resolve frame size.");
		return false;
	}

	//if (this->encoderFourCC == 0 && videoCapture)
	//	this->encoderFourCC = (int)videoCapture->get(cv::CAP_PROP_FOURCC);

	if (this->frameRateFPS == 0.0 && videoCapture)
		this->frameRateFPS = videoCapture->get(cv::CAP_PROP_FPS);

	this->videoWriter = new cv::VideoWriter();
	
	if (!this->videoWriter->open(*this->videoFilePath, this->encoderFourCC, this->frameRateFPS, *this->frameSize))
	{
		error.Add("Failed to open (for writing) the file: " + *this->videoFilePath);
		return false;
	}

	if (!this->videoWriter->isOpened())
	{
		error.Add("Failed to open video writer.");
		return false;
	}

	return true;
}

/*virtual*/ bool FileVideoDestination::PowerOff(Error& error)
{
	if (this->videoWriter)
	{
		this->videoWriter->release();
		delete this->videoWriter;
		this->videoWriter = nullptr;
	}

	return true;
}

/*virtual*/ bool FileVideoDestination::AddFrame(Frame& frame, Error& error)
{
	if (!this->videoWriter)
	{
		error.Add("No video writer to which we may add a frame.");
		return false;
	}

	//this->videoWriter->write(*frame.data);
	*this->videoWriter << *frame.data;
	return true;
}

void FileVideoDestination::SetVideoFilePath(const std::string& videoFilePath)
{
	*this->videoFilePath = videoFilePath;
}

const std::string& FileVideoDestination::GetVideoFilePath() const
{
	return *this->videoFilePath;
}