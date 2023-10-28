#include "FrameFilter.h"
#include "Error.h"

using namespace OpenVCR;

FrameFilter::FrameFilter()
{
	this->name = new std::string();
}

/*virtual*/ FrameFilter::~FrameFilter()
{
	delete this->name;
}

/*virtual*/ bool FrameFilter::Filter(Frame& inputFrame, Frame& outputFrame, Error& error)
{
	error.Add("Filter method not overridden.");
	return false;
}