#pragma once

#include "Common.h"
#include "Error.h"

namespace OpenVCR
{
	class IODevice;

	// TODO: OpenCV appears to leak memory.  Is it going to fill up RAM as we capture or playback?
	// Note that this class is not thread-safe.  If it's being used from multiple threads, then the
	// caller needs to handle thread-safety; e.g., with a mutex.
	class OPEN_VCR_API Machine
	{
	public:
		Machine();
		virtual ~Machine();

		static bool Setup(Error& error);
		static bool Shutdown(Error& error);

		bool PowerOn(Error& error);
		bool PowerOff(Error& error);
		bool Tick(Error& error);
		bool IsOn();

		template<typename IODeviceType>
		IODeviceType* AddIODevice(const std::string& name, Error& error)
		{
			if (this->isPoweredOn)
			{
				error.Add("Can't add IO device while machine is powered on.");
				return nullptr;
			}

			std::map<std::string, IODevice*>::iterator iter = this->ioDeviceMap->find(name);
			if (iter != this->ioDeviceMap->end())
			{
				error.Add(std::format("An IO device with name \"{}\" already exists.", name.c_str()));
				return nullptr;
			}

			IODeviceType* ioDevice = IODeviceType::Create(name);
			this->ioDeviceMap->insert(std::pair<std::string, IODevice*>(name, ioDevice));
			return ioDevice;
		}

		template<typename IODeviceType>
		IODeviceType* FindIODevice(const std::string& name)
		{
			std::map<std::string, IODevice*>::iterator iter = this->ioDeviceMap->find(name);
			if (iter == this->ioDeviceMap->end())
				return nullptr;

			return dynamic_cast<IODeviceType*>(iter->second);
		}

		template<typename IODeviceType>
		bool FindAllIODevices(std::vector<IODeviceType*>& ioDeviceArray)
		{
			for (auto pair : *this->ioDeviceMap)
			{
				IODeviceType* ioDevice = dynamic_cast<IODeviceType*>(pair.second);
				if (ioDevice)
					ioDeviceArray.push_back(ioDevice);
			}

			return ioDeviceArray.size() > 0;
		}

		bool DeleteAllIODevices(Error& error);

		void GetStatus(std::string& statusMsg);

		enum class Disposition
		{
			PULL,		// "Push" might be a more accurate term here.
			PLACE
		};

		void SetDisposition(Disposition disposition);
		Disposition GetDisposition() const;
		
		void SetPosition(double position);
		double GetPosition() const;

	private:

		std::map<std::string, IODevice*>* ioDeviceMap;
		bool isPoweredOn;
		Disposition disposition;
		double position;
	};
}