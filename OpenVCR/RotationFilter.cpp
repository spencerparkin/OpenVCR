#include "RotationFilter.h"
#include "Machine.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace OpenVCR;

RotationFilter::RotationFilter(const std::string& givenName) : VideoDevice(givenName)
{
	this->rotationAngleDegrees = 45.0;
}

/*virtual*/ RotationFilter::~RotationFilter()
{
}

/*static*/ RotationFilter* RotationFilter::Create(const std::string& name)
{
	return new RotationFilter(name);		// Allocate class in this DLL's heap!
}

/*virtual*/ bool RotationFilter::MoveData(Machine* machine, Error& error)
{
	VideoDevice* videoDevice = machine->FindIODevice<VideoDevice>(*this->sourceName);
	if (!videoDevice)
	{
		error.Add(std::format("Rotation filter failed to find video device of name \"{}\".", this->sourceName->c_str()));
		return false;
	}

	if (!videoDevice->IsComplete())
		return true;

	cv::Mat* sourceFrame = videoDevice->GetFrameData();
	if (!sourceFrame)
	{
		error.Add("Completed source did not have frame data for us.");
		return false;
	}
	
	cv::Point2f imageCenter(float(sourceFrame->cols) / 2.0f, float(sourceFrame->rows) / 2.0f);
	cv::Mat rotationMatrix = cv::getRotationMatrix2D(imageCenter, this->rotationAngleDegrees, 1.0);

	// TODO: Could we speed things up if the rotation is a multiple of 90 degrees?
	cv::warpAffine(*sourceFrame, *this->frame, rotationMatrix, sourceFrame->size());

	this->complete = true;
	return true;
}