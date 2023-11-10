#include "CropFilter.h"
#include "Machine.h"
#include "Error.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace OpenVCR;

CropFilter::CropFilter()
{
	this->cropParams = CropParams{ 0, 0, 0, 0 };
	this->sourceName = new std::string();
	this->frame = new cv::Mat();
	this->frameReady = false;
}

/*virtual*/ CropFilter::~CropFilter()
{
	delete this->sourceName;
	delete this->frame;
}

/*virtual*/ bool CropFilter::PreTick(Machine* machine, Error& error)
{
	this->frameReady = false;
	return true;
}

/*virtual*/ bool CropFilter::MoveData(Machine* machine, bool& moved, Error& error)
{
	moved = false;

	IODevice* ioDevice = machine->FindIODevice<IODevice>(*this->sourceName);
	if (!ioDevice)
	{
		error.Add(std::format("Crop filter failed to find IO device of name \"{}\".", this->sourceName->c_str()));
		return false;
	}

	cv::Mat* sourceFrame = ioDevice->GetFrameData();
	if (sourceFrame)
	{
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

		this->frameReady = true;
		moved = true;
	}

	return true;
}

/*virtual*/ cv::Mat* CropFilter::GetFrameData()
{
	if (this->frameReady)
		return this->frame;

	return nullptr;
}