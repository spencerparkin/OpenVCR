#pragma once

#include "Common.h"
#include "Frame.h"

namespace OpenVCR
{
	class Error;
	class VideoSource;
	class VideoDestination;

	// TODO: OpenCV appears to leak memory.  Is it going to fill up RAM as we capture or playback?
	class OPEN_VCR_API Machine
	{
	public:
		Machine();
		virtual ~Machine();

		bool PowerOn(Error& error);
		bool PowerOff(Error& error);
		bool Tick(Error& error);
		bool IsOn();

		bool SetVideoSource(VideoSource* videoSource, bool deleteToo, Error& error);
		VideoSource* GetVideoSource();

		bool AddVideoDestination(VideoDestination* videoDestination, Error& error);
		bool ClearAllVideoDestinations(bool deleteToo, Error& error);

		VideoDestination* GetVideoDestination(int i);
		int GetNumVideoDestination();

		void SetFrameRate(double frameRate);
		double GetFrameRate() const;

		void SetFramePosition(double framePosition);
		double GetFramePosition() const;

	private:
		VideoSource* videoSource;
		std::vector<VideoDestination*>* videoDestinationArray;
		// TODO: Add array of frame filters?
		Frame frame;
		bool isPoweredOn;
		double frameRate;
		double framePosition;
		clock_t lastClockTime;
	};
}