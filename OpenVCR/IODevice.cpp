#include "IODevice.h"

using namespace OpenVCR;

IODevice::IODevice()
{
	this->name = new std::string;
	this->sourceName = new std::string;
	this->complete = false;
}

/*virtual*/ IODevice::~IODevice()
{
	delete this->name;
	delete this->sourceName;
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
	this->complete = false;
	return true;
}

/*virtual*/ bool IODevice::PostTick(Machine* machine, Error& error)
{
	return true;
}

/*virtual*/ bool IODevice::MoveData(Machine* machine, Error& error)
{
	return false;
}

/*virtual*/ int IODevice::GetSortKey() const
{
	return 0;
}