#include "IODevice.h"
#include "Machine.h"

using namespace OpenVCR;

IODevice::IODevice(const std::string& givenName)
{
	this->name = new std::string;
	this->sourceNameArray = new std::vector<std::string>();
	this->complete = false;
	this->poweredOn = false;

	this->SetName(givenName);
}

/*virtual*/ IODevice::~IODevice()
{
	delete this->name;
	delete this->sourceNameArray;
}

/*virtual*/ bool IODevice::PowerOn(Machine* machine, Error& error)
{
	this->poweredOn = true;
	return true;
}

/*virtual*/ bool IODevice::PowerOff(Machine* machine, Error& error)
{
	this->poweredOn = false;
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

/*virtual*/ std::string IODevice::GetStatusMessage() const
{
	return "";
}

void IODevice::AddSourceName(const std::string& givenName)
{
	this->sourceNameArray->push_back(givenName);
}

std::string IODevice::GetSourceName(int i) const
{
	if (i < 0 || i >= (signed)this->sourceNameArray->size())
		return "";

	return (*this->sourceNameArray)[i];
}

int IODevice::GetNumSourceNames() const
{
	return (int)this->sourceNameArray->size();
}

void IODevice::ClearSourceNames()
{
	this->sourceNameArray->clear();
}

bool IODevice::AllSourcesPoweredOn(Machine& machine) const
{
	for (int i = 0; i < (signed)this->sourceNameArray->size(); i++)
	{
		IODevice* ioDevice = machine.FindIODevice<IODevice>(this->GetSourceName(i));
		if (!ioDevice || !ioDevice->IsPoweredOn())
			return false;
	}

	return true;
}