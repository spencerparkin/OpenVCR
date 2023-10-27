#include "VideoDestination.h"

using namespace OpenVCR;

VideoDestination::VideoDestination()
{
}

/*virtual*/ VideoDestination::~VideoDestination()
{
}

/*virtual*/ bool VideoDestination::PowerOn(Machine* machine, Error& error)
{
	return false;
}

/*virtual*/ bool VideoDestination::PowerOff(Error& error)
{
	return false;
}

/*virtual*/ bool VideoDestination::AddFrame(Frame& frame, Error& error)
{
	return false;
}