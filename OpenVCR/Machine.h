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
		bool IsOn();

		bool SetVideoSource(VideoSource* videoSource, bool deleteToo);
		VideoSource* GetVideoSource();

		bool AddVideoDestination(VideoDestination* videoDestination);
		bool ClearAllVideoDestinations(bool deleteToo);

		VideoDestination* GetVideoDestination(int i);
		int GetNumVideoDestination();

	private:
		VideoSource* videoSource;
		std::vector<VideoDestination*>* videoDestinationArray;
		Frame frame;
		bool isPoweredOn;
	};
}