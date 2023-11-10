#include "CropFilter.h"
#include "Machine.h"
#include "Error.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace OpenVCR;

CropFilter::CropFilter()
{
	this->cropParams = CropParams{ 0, 0, 0, 0 };
}

/*virtual*/ CropFilter::~CropFilter()
{
}

/*virtual*/ bool CropFilter::MoveData(Machine* machine, Error& error)
{
	VideoDevice* videoDevice = machine->FindIODevice<VideoDevice>(*this->sourceName);
	if (!videoDevice)
	{
		error.Add(std::format("Crop filter failed to find video device of name \"{}\".", this->sourceName->c_str()));
		return false;
	}

	if (!videoDevice->IsComplete())
		return true;

	cv::Mat* sourceFrame = videoDevice->GetFrameData();
	if (!sourceFrame)
	{
		error.Add("Completed source device did not have frame for us.");
		return false;
	}

	int width = sourceFrame->size().width - this->cropParams.leftCrop - this->cropParams.rightCrop;
	if (width <= 0)
	{
		error.Add("Crop killed all width!");
		return false;
	}

	int height = sourceFrame->size().height - this->cropParams.topCrop - this->cropParams.bottomCrop;
	if (height <= 0)
	{
		error.Add("Crop killed all height!");
		return false;
	}

	cv::Rect subRegion(this->cropParams.leftCrop, this->cropParams.topCrop, width, height);
	cv::Mat subFrame = (*sourceFrame)(subRegion);

	cv::resize(subFrame, *this->frame, sourceFrame->size());

	this->complete = true;
	return true;
}