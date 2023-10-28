#pragma once

#include "Common.h"
#include "Frame.h"

namespace OpenVCR
{
	class Error;
	class VideoSource;
	class VideoDestination;
	class FrameFilter;

	// TODO: OpenCV appears to leak memory.  Is it going to fill up RAM as we capture or playback?
	// Note that this class is not thread-safe.  If it's being used from multiple threads, then the
	// caller needs to handle thread-safety; e.g., with a mutex.
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

		bool AddFrameFilter(FrameFilter* frameFilter, Error& error);
		bool RemoveFrameFilter(const std::string& frameFilterName, bool deleteToo, Error& error);
		FrameFilter* FindFrameFilter(const std::string& frameFilterName, std::vector<FrameFilter*>::iterator* foundIter = nullptr);
		bool ClearAllFrameFilters(bool deleteToo, Error& error);

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
		std::vector<FrameFilter*>* frameFilterArray;
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