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
	this->pullMethod = SourcePullMethod::GET_NEXT_FRAME_QUICKEST;
	this->pullRatePPS = 30.0;
	this->framePosition = 0.0;
	this->frameRateFPP = 1.0;
	this->frameDirty = true;
	this->frameCleanTimeRemaining = 0.0;
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
	this->frameDirty = true;
	this->frameCleanTimeRemaining = 0.0;
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

	bool pushFrame = false;

	if (this->pullMethod == SourcePullMethod::GET_NEXT_FRAME_THROTTLED ||
		this->pullMethod == SourcePullMethod::SET_FRAME_POS_THROTTLED)
	{
		clock_t currentClockTime = ::clock();
		clock_t elapsedClockTime = currentClockTime - this->lastClockTime;
		double elapsedTimeSeconds = double(elapsedClockTime) / double(CLOCKS_PER_SEC);
		this->frameCleanTimeRemaining -= elapsedTimeSeconds;
		if (this->frameCleanTimeRemaining <= 0.0)
		{
			double secondsPerPull = 1.0 / this->pullRatePPS;
			while (this->frameCleanTimeRemaining <= 0.0)
				this->frameCleanTimeRemaining += secondsPerPull;
			this->frameDirty = true;
		}
	}

	if (this->pullMethod == SourcePullMethod::GET_NEXT_FRAME_QUICKEST ||
		(this->frameDirty && this->pullMethod == SourcePullMethod::GET_NEXT_FRAME_THROTTLED))
	{
		if (!this->videoSource->GetNextFrame(this->frame, error))
		{
			error.Add("Failed to get next frame.");
			return false;
		}

		pushFrame = true;
	}
	else if (this->pullMethod == SourcePullMethod::SET_FRAME_POS_QUICKEST ||
		(this->frameDirty &&
			(this->pullMethod == SourcePullMethod::SET_FRAME_POS_THROTTLED ||
				this->pullMethod == SourcePullMethod::SET_FRAME_POS_MANUAL)))
	{
		long i = (long)::floor(this->framePosition);
		if (!this->videoSource->GetFrame(this->frame, i, error))
		{
			error.Add(std::format("Failed to get frame {}.", i));
			return false;
		}

		if (this->pullMethod != SourcePullMethod::SET_FRAME_POS_MANUAL)
			this->framePosition += this->frameRateFPP;
		else
			this->frameDirty = false;

		pushFrame = true;
	}

	if (this->pullMethod != SourcePullMethod::SET_FRAME_POS_MANUAL)
		this->frameDirty = false;

	if (pushFrame)
	{
		// TODO: Run frame through filters here?

		for (VideoDestination* videoDestination : *this->videoDestinationArray)
			if (!videoDestination->AddFrame(this->frame, error))
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

void Machine::SetFrameRate(double frameRateFPP)
{
	this->frameRateFPP = frameRateFPP;
}

double Machine::GetFrameRate() const
{
	return this->frameRateFPP;
}

void Machine::SetFramePosition(double framePosition)
{
	this->framePosition = framePosition;
	this->frameDirty = true;
}

double Machine::GetFramePosition() const
{
	return this->framePosition;
}

void Machine::SetPullMethod(SourcePullMethod sourcePullMethod)
{
	this->pullMethod = sourcePullMethod;
}

Machine::SourcePullMethod Machine::GetPullMethod() const
{
	return this->pullMethod;
}

void Machine::GetStatus(std::string& statusMsg)
{
	if (!this->videoSource)
		statusMsg = "No video source.";
	else
	{
		Error error;

		long frameCount = 0;
		this->videoSource->GetFrameCount(frameCount, error);

		long frameNumber = 0;
		this->videoSource->GetFrameNumber(frameNumber, error);

		statusMsg = std::format("Frame: {}/{}", frameNumber, frameCount);
	}
}