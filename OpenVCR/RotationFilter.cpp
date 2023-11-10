#include "RotationFilter.h"
#include "Machine.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace OpenVCR;

RotationFilter::RotationFilter()
{
	this->sourceName = new std::string();
	this->rotationAngleDegrees = 45.0;
	this->frame = new cv::Mat();
	this->frameReady = false;
}

/*virtual*/ RotationFilter::~RotationFilter()
{
	delete this->sourceName;
	delete this->frame;
}

/*virtual*/ bool RotationFilter::PreTick(Machine* machine, Error& error)
{
	this->frameReady = false;
	return true;
}

/*virtual*/ bool RotationFilter::MoveData(Machine* machine, bool& moved, Error& error)
{
	moved = false;

	IODevice* ioDevice = machine->FindIODevice<IODevice>(*this->sourceName);
	if (!ioDevice)
	{
		error.Add(std::format("Rotation filter failed to find IO device of name \"{}\".", this->sourceName->c_str()));
		return false;
	}

	cv::Mat* sourceFrame = ioDevice->GetFrameData();
	if (sourceFrame)
	{
		cv::Point2f imageCenter(float(sourceFrame->cols) / 2.0f, float(sourceFrame->rows) / 2.0f);
		cv::Mat rotationMatrix = cv::getRotationMatrix2D(imageCenter, this->rotationAngleDegrees, 1.0);

		// TODO: Could we speed things up if the rotation is a multiple of 90 degrees?
		cv::warpAffine(*sourceFrame, *this->frame, rotationMatrix, sourceFrame->size());

		this->frameReady = true;
		moved = true;
	}

	return true;
}

/*virtual*/ cv::Mat* RotationFilter::GetFrameData()
{
	if (this->frameReady)
		return this->frame;

	return nullptr;
}