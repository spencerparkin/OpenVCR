#include "FileVideoDestination.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

FileVideoDestination::FileVideoDestination()
{
	this->videoWriter = nullptr;
	this->videoFilePath = new std::string();
	this->frameRateFPS = 30.0;
	this->encoderFourCC = 0;
	this->frameSize = new cv::Size(0, 0);
	this->sourceName = new std::string;
}

/*virtual*/ FileVideoDestination::~FileVideoDestination()
{
	delete this->videoFilePath;
	delete this->frameSize;
	delete this->sourceName;
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

	IODevice* ioDevice = machine->FindIODevice<IODevice>(*this->sourceName);
	if (!ioDevice)
	{
		error.Add(std::format("File video destination can't find frame source with name \"{}\".", this->sourceName->c_str()));
		return false;
	}

	if (!ioDevice->GetFrameSize(*this->frameSize, error))
	{
		error.Add("Could not query source device for frame size.");
		return false;
	}

	if (this->frameSize->width == 0 || this->frameSize->height == 0)
	{
		error.Add("Could not resolve frame size.");
		return false;
	}

	if (!ioDevice->GetFrameRate(this->frameRateFPS, error))
	{
		error.Add("Could not query for source device frame-rate.");
		return false;
	}

	// TODO: Need this?
	//this->encoderFourCC = (int)videoCapture->get(cv::CAP_PROP_FOURCC);

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

/*virtual*/ bool FileVideoDestination::MoveData(Machine* machine, bool& moved, Error& error)
{
	moved = false;

	if (!this->videoWriter)
	{
		error.Add("No video writer to which we may add a frame.");
		return false;
	}

	IODevice* ioDevice = machine->FindIODevice<IODevice>(*this->sourceName);
	if (!ioDevice)
	{
		error.Add(std::format("File video destination failed to find source IO device with name \"{}\".", this->sourceName->c_str()));
		return false;
	}

	cv::Mat* sourceFrame = ioDevice->GetFrameData();
	if (!sourceFrame)
		return true;

	if (!this->suspendFrameWrites)
	{
		//this->videoWriter->write(*sourceFrame);
		*this->videoWriter << *sourceFrame;
	}

	moved = true;
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