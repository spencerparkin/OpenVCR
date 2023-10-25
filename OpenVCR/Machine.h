#pragma once

#include "Common.h"
#include "Frame.h"

namespace OpenVCR
{
	class VideoSource;
	class VideoDestination;

	class OPEN_VCR_API Machine
	{
	public:
		Machine();
		virtual ~Machine();

		bool PowerOn();
		bool PowerOff();
		bool Tick();

		void SetVideoSource(VideoSource* videoSource);
		VideoSource* GetVideoSource();

		void AddVideoDestination(VideoDestination* videoDestination);
		void ClearAllVideoDestinations();

	private:
		VideoSource* videoSource;
		std::list<VideoDestination*>* videoDestinationList;
		Frame frame;
	};
}