#include "IODevice.h"

using namespace OpenVCR;

IODevice::IODevice()
{
	this->name = new std::string;
}

/*virtual*/ IODevice::~IODevice()
{
	delete this->name;
}

/*virtual*/ bool IODevice::PowerOn(Machine* machine, Error& error)
{
	return true;
}

/*virtual*/ bool IODevice::PowerOff(Machine* machine, Error& error)
{
	return true;
}

/*virtual*/ bool IODevice::PreTick(Machine* machine, Error& error)
{
	return true;
}

/*virtual*/ bool IODevice::PostTick(Machine* machine, Error& error)
{
	return true;
}

/*virtual*/ bool IODevice::MoveData(Machine* machine, bool& moved, Error& error)
{
	moved = false;
	return false;
}

/*virtual*/ cv::Mat* IODevice::GetFrameData()
{
	return nullptr;
}

/*virtual*/ void* IODevice::GetSampleData()
{
	return nullptr;
}

/*virtual*/ bool IODevice::GetFrameSize(cv::Size& frameSize, Error& error)
{
	return false;
}

/*virtual*/ bool IODevice::GetFrameRate(double& frameRate, Error& error)
{
	return false;
}

/*virtual*/ int IODevice::GetSortKey() const
{
	return 0;
}

void IODevice::SetName(const std::string& givenName)
{
	*this->name = givenName;
}

const std::string& IODevice::GetName() const
{
	return *this->name;
}