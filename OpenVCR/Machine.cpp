#include "Machine.h"
#include "VideoSource.h"
#include "VideoDestination.h"

using namespace OpenVCR;

Machine::Machine()
{
	this->isPoweredOn = false;
	this->videoSource = nullptr;
	this->videoDestinationArray = new std::vector<VideoDestination*>();
}

/*virtual*/ Machine::~Machine()
{
	delete this->videoDestinationArray;
}

bool Machine::IsOn()
{
	return this->isPoweredOn;
}

bool Machine::PowerOn()
{
	if (this->isPoweredOn)
		return false;

	if (!this->videoSource)
		return false;

	if (!this->videoSource->PowerOn())
		return false;

	for (VideoDestination* videoDestination : *this->videoDestinationArray)
		if (!videoDestination->PowerOn())
			return false;

	this->isPoweredOn = true;
	return true;
}

bool Machine::PowerOff()
{
	if (this->videoSource)
		this->videoSource->PowerOff();

	for (VideoDestination* videoDestination : *this->videoDestinationArray)
		videoDestination->PowerOff();

	this->isPoweredOn = false;
	return true;
}

bool Machine::Tick()
{
	// TODO: Control the frame-rate.  For now, just pump frames as fast as possible.

	if (!this->videoSource)
		return false;

	if (!this->videoSource->GetNextFrame(frame))
		return false;

	for (VideoDestination* videoDestination : *this->videoDestinationArray)
		if (!videoDestination->AddFrame(frame))
			return false;

	return true;
}

bool Machine::SetVideoSource(VideoSource* videoSource, bool deleteToo)
{
	if (this->isPoweredOn)
		return false;

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

bool Machine::AddVideoDestination(VideoDestination* videoDestination)
{
	if (this->isPoweredOn)
		return false;

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

bool Machine::ClearAllVideoDestinations(bool deleteToo)
{
	if (this->isPoweredOn)
		return false;

	if (deleteToo)
		for (VideoDestination* videoDestion : *this->videoDestinationArray)
			delete videoDestion;

	this->videoDestinationArray->clear();
	return true;
}