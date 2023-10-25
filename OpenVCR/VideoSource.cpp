#include "VideoSource.h"

using namespace OpenVCR;

VideoSource::VideoSource()
{
}

/*virtual*/ VideoSource::~VideoSource()
{
}

/*virtual*/ bool VideoSource::GetFrameCount(int& frameCount)
{
	return false;
}

/*virtual*/ bool VideoSource::GetFrame(Frame& frame, int i)
{
	return false;
}

/*virtual*/ bool VideoSource::GetNextFrame(Frame& frame)
{
	return false;
}