#include "CropFilter.h"
#include "Frame.h"
#include "Error.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace OpenVCR;

CropFilter::CropFilter()
{
	this->leftCrop = 0;
	this->rightCrop = 0;
	this->topCrop = 0;
	this->bottomCrop = 0;
}

/*virtual*/ CropFilter::~CropFilter()
{
}

/*virtual*/ bool CropFilter::Filter(Frame& inputFrame, Frame& outputFrame, Error& error)
{
	int width = inputFrame.data->size().width - this->leftCrop - this->rightCrop;
	if (width <= 0)
	{
		error.Add("Crop killed all width!");
		return false;
	}

	int height = inputFrame.data->size().height - this->topCrop - this->bottomCrop;
	if (height <= 0)
	{
		error.Add("Crop killed all height!");
		return false;
	}

	cv::Rect subRegion(this->leftCrop, this->topCrop, width, height);
	cv::Mat subFrame = (*inputFrame.data)(subRegion);

	cv::resize(subFrame, *outputFrame.data, inputFrame.data->size());

	return true;
}