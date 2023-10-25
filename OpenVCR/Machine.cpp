#include "Machine.h"
#include "VideoSource.h"
#include "VideoDestination.h"

using namespace OpenVCR;

Machine::Machine()
{
	this->videoSource = nullptr;
	this->videoDestinationList = new std::list<VideoDestination*>();
}

/*virtual*/ Machine::~Machine()
{
	delete this->videoDestinationList;
}

bool Machine::PowerOn()
{
	return false;
}

bool Machine::PowerOff()
{
	return false;
}

bool Machine::Tick()
{
	// TODO: Control the frame-rate.  For now, just pump frames as fast as possible.

	if (!this->videoSource)
		return false;

	if (!this->videoSource->GetNextFrame(frame))
		return false;

	for (VideoDestination* videoDestination : *this->videoDestinationList)
		if (!videoDestination->AddFrame(frame))
			return false;

	return true;
}

void Machine::SetVideoSource(VideoSource* videoSource)
{
	this->videoSource = videoSource;
}

VideoSource* Machine::GetVideoSource()
{
	return this->videoSource;
}

void Machine::AddVideoDestination(VideoDestination* videoDestination)
{
	this->videoDestinationList->push_back(videoDestination);
}

void Machine::ClearAllVideoDestinations()
{
	this->videoDestinationList->clear();
}