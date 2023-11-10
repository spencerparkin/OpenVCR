#include "FileVideoDestination.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

FileVideoDestination::FileVideoDestination(const std::string& givenName) : VideoDevice(givenName)
{
	this->videoWriter = nullptr;
	this->videoFilePath = new std::string();
}

/*virtual*/ FileVideoDestination::~FileVideoDestination()
{
	delete this->videoFilePath;
}

/*static*/ FileVideoDestination* FileVideoDestination::Create(const std::string& name)
{
	return new FileVideoDestination(name);		// Allocate class in this DLL's heap!
}

/*virtual*/ int FileVideoDestination::GetSortKey() const
{
	// This is a bit hacky, I guesss, but the idea here is to make sure that
	// we get powered-on *after* everything else we care about.
	return 1;
}

/*virtual*/ bool FileVideoDestination::PowerOn(Machine* machine, Error& error)
{
	if (this->videoWriter)
	{
		error.Add("Video writer already created.");
		return false;
	}

	VideoDevice* videoDevice = machine->FindIODevice<VideoDevice>(*this->sourceName);
	if (!videoDevice)
	{
		error.Add(std::format("File video destination can't find frame source with name \"{}\".", this->sourceName->c_str()));
		return false;
	}

	cv::Size frameSize;
	if (!videoDevice->GetFrameSize(frameSize, error))
	{
		error.Add("Could not query source device for frame size.");
		return false;
	}

	if (frameSize.width == 0 || frameSize.height == 0)
	{
		frameSize.width = 512;
		frameSize.height = 512;
	}

	double frameRateFPS = 30.0;
	if (!videoDevice->GetFrameRate(frameRateFPS, error))
	{
		error.Add("Could not query for source device frame-rate.");
		return false;
	}

	if (frameRateFPS == 0)
		frameRateFPS = 30.0f;

	int encoderFourCC = 0;
	// TODO: Need this?
	//encoderFourCC = (int)videoCapture->get(cv::CAP_PROP_FOURCC);

	this->videoWriter = new cv::VideoWriter();
	
	if (!this->videoWriter->open(*this->videoFilePath, encoderFourCC, frameRateFPS, frameSize))
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

/*virtual*/ bool FileVideoDestination::PowerOff(Machine* machine, Error& error)
{
	if (this->videoWriter)
	{
		this->videoWriter->release();
		delete this->videoWriter;
		this->videoWriter = nullptr;
	}

	return true;
}

/*virtual*/ bool FileVideoDestination::MoveData(Machine* machine, Error& error)
{
	if (!this->videoWriter)
	{
		error.Add("No video writer to which we may add a frame.");
		return false;
	}

	VideoDevice* videoDevice = machine->FindIODevice<VideoDevice>(*this->sourceName);
	if (!videoDevice)
	{
		error.Add(std::format("File video destination failed to find source IO device with name \"{}\".", this->sourceName->c_str()));
		return false;
	}

	if (!videoDevice->IsComplete())
		return true;

	cv::Mat* sourceFrame = videoDevice->GetFrameData();
	if (!sourceFrame)
	{
		error.Add("Completed source did not have any frame data for us.");
		return false;
	}

	if (!this->suspendFrameWrites)
	{
		//this->videoWriter->write(*sourceFrame);
		*this->videoWriter << *sourceFrame;
	}

	this->complete = true;
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

void FileVideoDestination::Pause()
{
	this->suspendFrameWrites = true;
}

void FileVideoDestination::Resume()
{
	this->suspendFrameWrites = false;
}

bool FileVideoDestination::IsPaused() const
{
	return this->suspendFrameWrites;
}