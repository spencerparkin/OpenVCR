#include "Frame.h"

using namespace OpenVCR;

Frame::Frame()
{
	this->data = new cv::Mat();
}

/*virtual*/ Frame::~Frame()
{
	delete this->data;
}