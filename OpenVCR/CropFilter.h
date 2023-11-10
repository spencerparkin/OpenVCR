#pragma once

#include "IODevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API CropFilter : public IODevice
	{
	public:
		CropFilter();
		virtual ~CropFilter();

		virtual bool PreTick(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, bool& moved, Error& error) override;
		virtual cv::Mat* GetFrameData() override;

		struct CropParams
		{
			int leftCrop;
			int rightCrop;
			int topCrop;
			int bottomCrop;
		};

		void SetSourceName(const std::string& givenName) { *this->sourceName = givenName; }
		const std::string& GetSourceName() const { return *this->sourceName; }

		void SetCropParams(const CropParams& cropParams) { this->cropParams = cropParams; }
		const CropParams& GetCropParams() const { return this->cropParams; }

	private:
		CropParams cropParams;
		cv::Mat* frame;
		bool frameReady;
		std::string* sourceName;
	};
}