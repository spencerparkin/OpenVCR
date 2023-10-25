#pragma once

#include "VideoSource.h"

namespace OpenVCR
{
	class OPEN_VCR_API FileVideoSource : public VideoSource
	{
	public:
		FileVideoSource();
		virtual ~FileVideoSource();

		void SetVideoFilePath(const std::string& videoFilePath);
		const std::string& GetVideoFilePath();

		virtual bool GetFrameCount(int& frameCount) override;
		virtual bool GetFrame(Frame& frame, int i) override;
		virtual bool GetNextFrame(Frame& frame) override;

	private:

		std::string* videoFilePath;
	};
}