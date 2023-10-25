#include "VideoDestination.h"

using namespace OpenVCR;

VideoDestination::VideoDestination()
{
}

/*virtual*/ VideoDestination::~VideoDestination()
{
}

/*virtual*/ bool VideoDestination::PowerOn()
{
	return false;
}

/*virtual*/ bool VideoDestination::PowerOff()
{
	return false;
}

/*virtual*/ bool VideoDestination::AddFrame(Frame& frame)
{
	return false;
}