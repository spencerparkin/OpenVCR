#pragma once

#include "VideoDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API CropFilter : public VideoDevice
	{
	public:
		CropFilter(const std::string& givenName);
		virtual ~CropFilter();

		static CropFilter* Create(const std::string& name);

		virtual bool MoveData(Machine* machine, Error& error) override;

		struct CropParams
		{
			int leftCrop;
			int rightCrop;
			int topCrop;
			int bottomCrop;
		};

		void SetCropParams(const CropParams& cropParams) { this->cropParams = cropParams; }
		const CropParams& GetCropParams() const { return this->cropParams; }

	private:
		CropParams cropParams;
	};
}