#include "Machine.h"
#include "Error.h"
#include "VideoSource.h"
#include "VideoDestination.h"
#include "FrameFilter.h"
#include <opencv2/core/utils/logger.hpp>

using namespace OpenVCR;

Machine::Machine()
{
	this->isPoweredOn = false;
	this->videoSource = nullptr;
	this->videoDestinationArray = new std::vector<VideoDestination*>();
	this->frameFilterArray = new std::vector<FrameFilter*>();
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
	delete this->frameFilterArray;
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

#if defined _DEBUG
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_VERBOSE);
#else
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
#endif

	if (!this->videoSource->PowerOn(error))
	{
		error.Add("Video source power-on failed.");
		return false;
	}

	for (VideoDestination* videoDestination : *this->videoDestinationArray)
	{
		if (!videoDestination->PowerOn(this, error))
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
		if (this->frame.data->empty())
		{
			error.Add("Can't push empty frame.");
			return false;
		}

		// Filter the generated frame through all the filters, if any.
		Frame otherFrame;
		Frame* doubleBuffer[2] = { &this->frame, &otherFrame };
		int i = 0;
		for (FrameFilter* frameFilter : *this->frameFilterArray)
		{
			Frame* inputFrame = doubleBuffer[i];
			Frame* outputFrame = doubleBuffer[1 - i];

			if (!frameFilter->Filter(*inputFrame, *outputFrame, error))
			{
				error.Add("Filter \"" + *frameFilter->name + "\" failed!");
				return false;
			}

			i = 1 - i;
		}

		// Send the final frame to all the video destinations.
		Frame* finalFrame = doubleBuffer[i];
		for (VideoDestination* videoDestination : *this->videoDestinationArray)
			if (!videoDestination->AddFrame(*finalFrame, error))
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

bool Machine::AddFrameFilter(FrameFilter* frameFilter, Error& error)
{
	FrameFilter* existingFilter = this->FindFrameFilter(*frameFilter->name);
	if (existingFilter)
	{
		error.Add("There already exists a filter with name: " + *frameFilter->name);
		return false;
	}

	this->frameFilterArray->push_back(frameFilter);
	return true;
}

bool Machine::RemoveFrameFilter(const std::string& frameFilterName, bool deleteToo, Error& error)
{
	std::vector<FrameFilter*>::iterator iter;
	FrameFilter* frameFilter = this->FindFrameFilter(frameFilterName, &iter);
	if (!frameFilter)
	{
		error.Add("Did not find frame filter with name: " + frameFilterName);
		return false;
	}

	if (deleteToo)
		delete frameFilter;

	this->frameFilterArray->erase(iter);
	return true;
}

FrameFilter* Machine::FindFrameFilter(const std::string& frameFilterName, std::vector<FrameFilter*>::iterator* foundIter /*= nullptr*/)
{
	for (std::vector<FrameFilter*>::iterator iter = this->frameFilterArray->begin(); iter != this->frameFilterArray->end(); iter++)
	{
		FrameFilter* frameFilter = *iter;
		if (*frameFilter->name == frameFilterName)
		{
			if (foundIter)
				*foundIter = iter;

			return frameFilter;
		}
	}

	return nullptr;
}

bool Machine::ClearAllFrameFilters(bool deleteToo, Error& error)
{
	if (deleteToo)
		for (FrameFilter* frameFilter : *this->frameFilterArray)
			delete frameFilter;

	this->frameFilterArray->clear();
	return true;
}

void Machine::SetPullRate(double pullRate)
{
	this->pullRatePPS = pullRate;
}

double Machine::GetPullRate() const
{
	return this->pullRatePPS;
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