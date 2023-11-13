#pragma once

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/slider.h>
#include "Thread.h"

class Thread;

class Frame : public wxFrame
{
public:
	Frame();
	virtual ~Frame();

	enum
	{
		ID_SetupToCaptureVideo,
		ID_SetupToReplayVideo,
		ID_SetupToCaptureAudio,
		ID_SetupToReplayAudio,
		ID_PowerOnMachine,
		ID_PowerOffMachine,
		ID_ScrubMode,
		ID_About,
		ID_Exit
	};

	void OnSetupMachine(wxCommandEvent& event);
	void OnPowerMachine(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnThreadEntering(wxThreadEvent& event);
	void OnThreadExiting(wxThreadEvent& event);
	void OnThreadError(ThreadErrorEvent& event);
	void OnThreadStatus(ThreadStatusEvent& event);
	void OnSliderChanged(wxScrollEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnScrubMode(wxCommandEvent& event);

	void StartThread();
	void StopThread();

	bool StringToNumberArray(const wxString& str, std::vector<int>& numberArray);

	Thread* thread;
	wxSlider* slider;
	wxControl* renderControl;
	bool scrubMode;
};