#pragma once

#include "Common.h"

namespace OpenVCR
{
	class OPEN_VCR_API Error
	{
	public:
		Error();
		virtual ~Error();

		void Add(const std::string& errorMsg);
		std::string GetErrorMessage() const;
		int GetCount() const;
		void Clear();

	private:
		std::vector<std::string>* errorArray;
	};
}