#include "IODevice.h"

using namespace OpenVCR;

IODevice::IODevice(const std::string& givenName)
{
	this->name = new std::string;
	this->sourceName = new std::string;
	this->complete = false;

	this->SetName(givenName);
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

/*virtual*/ std::string IODevice::GetStatusMessage() const
{
	return "";
}