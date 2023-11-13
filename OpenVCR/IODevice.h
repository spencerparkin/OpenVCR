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
		virtual int GetSortKey() const;
		virtual std::string GetStatusMessage() const;
		
		void SetName(const std::string& givenName) { *this->name = givenName; }
		const std::string& GetName() const { return *this->name; }

		void SetSourceName(const std::string& givenName) { *this->sourceName = givenName; }
		const std::string& GetSourceName() const { return *this->sourceName; }

		bool IsComplete() const { return this->complete; }

	protected:

		bool complete;
		std::string* name;
		std::string* sourceName;
	};
}