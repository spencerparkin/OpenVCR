#include "VideoDestination.h"

using namespace OpenVCR;

VideoDestination::VideoDestination()
{
}

/*virtual*/ VideoDestination::~VideoDestination()
{
}

bool VideoDestination::AddFrame(Frame& frame)
{
	return false;
}