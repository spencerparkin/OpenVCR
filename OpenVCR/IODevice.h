#pragma once

#include "Common.h"
#include <opencv2/core/mat.hpp>

namespace OpenVCR
{
	class Error;
	class Machine;

	class OPEN_VCR_API IODevice
	{
	public:
		IODevice(const std::string& givenName);
		virtual ~IODevice();

		virtual bool PowerOn(Machine* machine, Error& error);
		virtual bool PowerOff(Machine* machine, Error& error);
		virtual bool PreTick(Machine* machine, Error& error);
		virtual bool PostTick(Machine* machine, Error& error);
		virtual bool MoveData(Machine* machine, Error& error);
		virtual std::string GetStatusMessage() const;
		
		void SetName(const std::string& givenName) { *this->name = givenName; }
		const std::string& GetName() const { return *this->name; }

		void AddSourceName(const std::string& givenName);
		std::string GetSourceName(int i) const;
		int GetNumSourceNames() const;
		void ClearSourceNames();

		bool IsPoweredOn() const { return this->poweredOn; }
		bool IsPoweredOff() const { return !this->poweredOn; }
		bool IsComplete() const { return this->complete; }
		
		bool AllSourcesPoweredOn(Machine& machine) const;

	protected:

		bool poweredOn;
		bool complete;
		std::string* name;
		std::vector<std::string>* sourceNameArray;
	};
}