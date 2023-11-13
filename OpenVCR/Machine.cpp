#include "Machine.h"
#include "Error.h"
#include "IODevice.h"
#include <opencv2/core/utils/logger.hpp>
#include <SDL.h>

using namespace OpenVCR;

Machine::Machine()
{
	this->isPoweredOn = false;
	this->ioDeviceMap = new std::map<std::string, IODevice*>();
	this->disposition = Disposition::PULL;
	this->position = 0.0;
}

/*virtual*/ Machine::~Machine()
{
	delete this->ioDeviceMap;
}

/*static*/ bool Machine::Setup(Error& error)
{
	int result = SDL_Init(SDL_INIT_AUDIO);
	if (result != 0)
	{
		error.Add(std::format("Failed to initialize SDL audo subsystem: {}", SDL_GetError()));
		return false;
	}

	return true;
}

/*static*/ bool Machine::Shutdown(Error& error)
{
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	SDL_Quit();

	return true;
}

bool Machine::DeleteAllIODevices(Error& error)
{
	if (this->isPoweredOn)
	{
		error.Add("Can't delete all IO devices while machine is powered-on.");
		return false;
	}

	for (auto pair : *this->ioDeviceMap)
		delete pair.second;

	this->ioDeviceMap->clear();
	return true;
}

bool Machine::IsOn()
{
	return this->isPoweredOn;
}

bool Machine::PowerOn(Error& error)
{
	if (this->isPoweredOn)
	{
		error.Add("Already powered on.");
		return false;
	}

#if defined _DEBUG
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_VERBOSE);
#else
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
#endif

	std::vector<IODevice*> ioDeviceArray;
	for (auto pair : *this->ioDeviceMap)
		ioDeviceArray.push_back(pair.second);

	std::sort(ioDeviceArray.begin(), ioDeviceArray.end(), [](const IODevice* deviceA, const IODevice* deviceB) -> int
		{
			return deviceA->GetSortKey() < deviceB->GetSortKey();
		});

	for (IODevice* ioDevice : ioDeviceArray)
	{
		if (!ioDevice->PowerOn(this, error))
		{
			error.Add("IO device power-on failed.");
			return false;
		}
	}

	this->isPoweredOn = true;
	return true;
}

bool Machine::PowerOff(Error& error)
{
	for (auto pair : *this->ioDeviceMap)
	{
		IODevice* ioDevice = pair.second;
		ioDevice->PowerOff(this, error);
	}

	this->isPoweredOn = false;
	return error.GetCount() == 0;
}

bool Machine::Tick(Error& error)
{
	if (!this->isPoweredOn)
	{
		error.Add("Machine can't be ticked if it isn't on.");
		return false;
	}

	for (auto pair : *this->ioDeviceMap)
	{
		IODevice* ioDevice = pair.second;
		if (!ioDevice->PreTick(this, error))
		{
			error.Add("Pre-tick on IO device failed.");
			return false;
		}
	}

	std::list<IODevice*> ioDeviceQueue;
	for (auto pair : *this->ioDeviceMap)
		ioDeviceQueue.push_back(pair.second);

	int deferCount = 0;
	while (ioDeviceQueue.size() > 0)
	{
		std::list<IODevice*>::iterator iter = ioDeviceQueue.begin();
		IODevice* ioDevice = *iter;
		ioDeviceQueue.erase(iter);

		if (!ioDevice->MoveData(this, error))
		{
			error.Add("IO device failed when trying to move data.");
			return false;
		}

		if (ioDevice->IsComplete())
			deferCount = 0;
		else
		{
			ioDeviceQueue.push_back(ioDevice);
			deferCount++;
		}

		if (deferCount == ioDeviceQueue.size() && deferCount > 0)
		{
			error.Add("All IO devices are deferring the movement of data.  Infinite loop detected.");
			return false;
		}
	}

	for (auto pair : *this->ioDeviceMap)
	{
		IODevice* ioDevice = pair.second;
		if (!ioDevice->PostTick(this, error))
		{
			error.Add("Post-tick on IO device failed.");
			return false;
		}
	}

	return true;
}

void Machine::GetStatus(std::string& statusMsg)
{
	statusMsg = "";

	for (auto pair : *this->ioDeviceMap)
	{
		IODevice* ioDevice = pair.second;

		std::string deviceMsg = ioDevice->GetStatusMessage();
		if (deviceMsg.length() > 0)
		{
			if (statusMsg.length() > 0)
				statusMsg += " / ";
			statusMsg += deviceMsg;
		}
	}
}

Machine::Disposition Machine::GetDisposition() const
{
	return this->disposition;
}

void Machine::SetDisposition(Disposition disposition)
{
	this->disposition = disposition;
}

void Machine::SetPosition(double position)
{
	this->position = position;

	if (this->position < 0.0)
		this->position = 0.0;
	else if (this->position > 1.0)
		this->position = 1.0;
}

double Machine::GetPosition() const
{
	return this->position;
}