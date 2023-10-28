#include "RotationFilter.h"
#include "Frame.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace OpenVCR;

RotationFilter::RotationFilter()
{
	this->rotationAngleDegrees = 45.0;
}

/*virtual*/ RotationFilter::~RotationFilter()
{
}

/*virtual*/ bool RotationFilter::Filter(Frame& inputFrame, Frame& outputFrame, Error& error)
{
	cv::Point2f imageCenter(float(inputFrame.data->cols) / 2.0f, float(inputFrame.data->rows) / 2.0f);
	cv::Mat rotationMatrix = cv::getRotationMatrix2D(imageCenter, this->rotationAngleDegrees, 1.0);

	// TODO: Could we speed things up if the rotation is a multiple of 90 degrees?
	cv::warpAffine(*inputFrame.data, *outputFrame.data, rotationMatrix, inputFrame.data->size());

	return true;
}