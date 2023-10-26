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

		enum SourcePullMethod
		{
			GET_NEXT_FRAME_QUICKEST,
			GET_NEXT_FRAME_THROTTLED,
			SET_FRAME_POS_QUICKEST,
			SET_FRAME_POS_THROTTLED,
			SET_FRAME_POS_MANUAL
		};

		void SetPullMethod(SourcePullMethod sourcePullmethod);
		SourcePullMethod GetPullMethod() const;

		void SetPullRate(double pullRate);
		double GetPullRate() const;

		void SetFrameRate(double frameRateFPP);
		double GetFrameRate() const;

		void SetFramePosition(double framePosition);
		double GetFramePosition() const;

		void GetStatus(std::string& statusMsg);

	private:
		VideoSource* videoSource;
		std::vector<VideoDestination*>* videoDestinationArray;
		// TODO: Add array of frame filters?
		Frame frame;
		bool isPoweredOn;
		SourcePullMethod pullMethod;
		double pullRatePPS;			// Pulls per second.
		double frameRateFPP;		// Frames per pull.
		double framePosition;
		double frameCleanTimeRemaining;
		bool frameDirty;
		clock_t lastClockTime;
	};
}