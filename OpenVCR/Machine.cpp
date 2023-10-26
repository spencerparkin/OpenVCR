#include "Machine.h"
#include "Error.h"
#include "VideoSource.h"
#include "VideoDestination.h"

using namespace OpenVCR;

Machine::Machine()
{
	this->isPoweredOn = false;
	this->videoSource = nullptr;
	this->videoDestinationArray = new std::vector<VideoDestination*>();
	this->lastClockTime = 0;
	this->frameRate = 30.0;
	this->framePosition = 0.0;
}

/*virtual*/ Machine::~Machine()
{
	delete this->videoDestinationArray;
}

bool Machine::IsOn()
{
	return this->isPoweredOn;
}

bool Machine::PowerOn(Error& error)
{
	if (this->isPoweredOn)
	{
		error.Add("Already powered on.");
		return false;
	}

	if (!this->videoSource)
	{
		error.Add("No video source configured.");
		return false;
	}

	if (!this->videoSource->PowerOn(error))
	{
		error.Add("Video source power-on failed.");
		return false;
	}

	for (VideoDestination* videoDestination : *this->videoDestinationArray)
	{
		if (!videoDestination->PowerOn(error))
		{
			error.Add("Video destination power-on failed.");
			return false;
		}
	}

	this->lastClockTime = ::clock();
	this->framePosition = 0.0;
	this->isPoweredOn = true;
	return true;
}

bool Machine::PowerOff(Error& error)
{
	if (this->videoSource)
		this->videoSource->PowerOff(error);

	for (VideoDestination* videoDestination : *this->videoDestinationArray)
		videoDestination->PowerOff(error);

	this->isPoweredOn = false;
	return true;
}

bool Machine::Tick(Error& error)
{
	if (!this->videoSource)
	{
		error.Add("No frame source configured!");
		return false;
	}

	clock_t currentClockTime = ::clock();
	clock_t elapsedClockTime = currentClockTime - this->lastClockTime;
	this->lastClockTime = currentClockTime;
	double elapsedTimeSec = double(elapsedClockTime) / double(CLOCKS_PER_SEC);
	double nextFramePosition = this->framePosition + this->frameRate * elapsedTimeSec;
	bool passedIntegerBoundary = (::floor(nextFramePosition) != ::floor(this->framePosition));
	this->framePosition = nextFramePosition;
	if (passedIntegerBoundary)
	{
		if (this->videoSource->IsLiveStream())
		{
			if (!this->videoSource->GetNextFrame(frame, error))
			{
				error.Add("Failed to get next frame.");
				return false;
			}
		}
		else
		{
			long i = (long)::floor(this->framePosition);
			
			long frameCount = 0;
			if (!this->videoSource->GetFrameCount(frameCount, error))
			{
				error.Add("Could not determine video source frame count.");
				return false;
			}

			if (i < 0)
				i = 0;
			else if (i >= frameCount)
				i = frameCount - 1;

			if (!this->videoSource->GetFrame(frame, i, error))
			{
				error.Add(std::format("Failed to get frame {} of {} total frames.", i, frameCount));
				return false;
			}
		}

		// TODO: Run frame through filters here?

		for (VideoDestination* videoDestination : *this->videoDestinationArray)
			if (!videoDestination->AddFrame(frame, error))
				return false;
	}

	return true;
}

bool Machine::SetVideoSource(VideoSource* videoSource, bool deleteToo, Error& error)
{
	if (this->isPoweredOn)
	{
		error.Add("Can't configure video source when already powered on.");
		return false;
	}

	if (this->videoSource)
	{
		if (deleteToo)
			delete this->videoSource;
	}

	this->videoSource = videoSource;
	return true;
}

VideoSource* Machine::GetVideoSource()
{
	return this->videoSource;
}

bool Machine::AddVideoDestination(VideoDestination* videoDestination, Error& error)
{
	if (this->isPoweredOn)
	{
		error.Add("Can't configure video destination when already powered on.");
		return false;
	}

	this->videoDestinationArray->push_back(videoDestination);
	return true;
}

VideoDestination* Machine::GetVideoDestination(int i)
{
	if (0 <= i && i < (int)this->videoDestinationArray->size())
		return (*this->videoDestinationArray)[i];

	return nullptr;
}

int Machine::GetNumVideoDestination()
{
	return (int)this->videoDestinationArray->size();
}

bool Machine::ClearAllVideoDestinations(bool deleteToo, Error& error)
{
	if (this->isPoweredOn)
	{
		error.Add("Can't clear video destination configurationw hen already powered on.");
		return false;
	}

	if (deleteToo)
		for (VideoDestination* videoDestion : *this->videoDestinationArray)
			delete videoDestion;

	this->videoDestinationArray->clear();
	return true;
}

void Machine::SetFrameRate(double frameRate)
{
	this->frameRate = frameRate;
}

double Machine::GetFrameRate() const
{
	return this->frameRate;
}

void Machine::SetFramePosition(double framePosition)
{
	this->framePosition = framePosition;
}

double Machine::GetFramePosition() const
{
	return this->framePosition;
}