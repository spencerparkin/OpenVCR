#include "FileVideoDestination.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

FileVideoDestination::FileVideoDestination(const std::string& givenName) : VideoDevice(givenName)
{
	this->videoWriter = nullptr;
	this->videoFilePath = new std::string();
	this->frameSize = new cv::Size(0, 0);
	this->frameRateFPS = 0.0;
	this->suspendFrameWrites = false;
}

/*virtual*/ FileVideoDestination::~FileVideoDestination()
{
	delete this->videoFilePath;
	delete this->frameSize;
}

/*static*/ FileVideoDestination* FileVideoDestination::Create(const std::string& name)
{
	return new FileVideoDestination(name);		// Allocate class in this DLL's heap!
}

/*virtual*/ bool FileVideoDestination::PowerOn(Machine* machine, Error& error)
{
	if (this->videoWriter)
	{
		error.Add("Video writer already created.");
		return false;
	}

	if (this->GetNumSourceNames() != 1)
	{
		error.Add("File video destination expected exactly one source.");
		return false;
	}

	VideoDevice* videoDevice = machine->FindIODevice<VideoDevice>(this->GetSourceName(0));
	if (!videoDevice)
	{
		error.Add(std::format("File video destination can't find frame source with name \"{}\".", this->GetSourceName(0).c_str()));
		return false;
	}

	if (this->frameSize->width == 0 || this->frameSize->height == 0)
	{
		if (!videoDevice->GetFrameSize(*this->frameSize, error))
		{
			error.Add("Could not query source device for frame size.");
			return false;
		}
	}

	if (this->frameRateFPS == 0.0)
	{
		if (!videoDevice->GetFrameRate(this->frameRateFPS, error))
		{
			error.Add("Could not query for source device frame-rate.");
			return false;
		}
	}

	if (this->frameRateFPS == 0.0)
		this->frameRateFPS = 30.0;

	int encoderFourCC = 0;
	// TODO: Need this?
	//encoderFourCC = (int)videoCapture->get(cv::CAP_PROP_FOURCC);

	this->videoWriter = new cv::VideoWriter();
	
	if (!this->videoWriter->open(*this->videoFilePath, encoderFourCC, this->frameRateFPS, *this->frameSize))
	{
		error.Add("Failed to open (for writing) the file: " + *this->videoFilePath);
		return false;
	}

	if (!this->videoWriter->isOpened())
	{
		error.Add("Failed to open video writer.");
		return false;
	}

	this->poweredOn = true;
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

	this->poweredOn = false;
	return true;
}

void FileVideoDestination::SetFrameSize(int width, int height)
{
	this->frameSize->width = width;
	this->frameSize->height = height;
}

void FileVideoDestination::SetFrameRate(double frameRateFPS)
{
	this->frameRateFPS = frameRateFPS;
}

/*virtual*/ bool FileVideoDestination::MoveData(Machine* machine, Error& error)
{
	if (this->GetNumSourceNames() != 1)
	{
		error.Add("File video destination expected exactly one source.");
		return false;
	}

	VideoDevice* videoDevice = machine->FindIODevice<VideoDevice>(this->GetSourceName(0));
	if (!videoDevice)
	{
		error.Add(std::format("File video destination failed to find source IO device with name \"{}\".", this->GetSourceName(0).c_str()));
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