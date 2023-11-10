#pragma once

#include "VideoDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API CropFilter : public VideoDevice
	{
	public:
		CropFilter();
		virtual ~CropFilter();

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