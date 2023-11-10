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
		IODevice();
		virtual ~IODevice();

		virtual bool PowerOn(Machine* machine, Error& error);
		virtual bool PowerOff(Machine* machine, Error& error);
		virtual bool PreTick(Machine* machine, Error& error);
		virtual bool PostTick(Machine* machine, Error& error);
		virtual bool MoveData(Machine* machine, bool& moved, Error& error);
		virtual cv::Mat* GetFrameData();
		virtual void* GetSampleData();
		virtual bool GetFrameSize(cv::Size& frameSize, Error& error);
		virtual bool GetFrameRate(double& frameRate, Error& error);
		virtual int GetSortKey() const;
		
		void SetName(const std::string& givenName);
		const std::string& GetName() const;

	protected:

		std::string* name;
	};
}