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
		ID_SetFileVideoSource = wxID_HIGHEST,
		ID_SetCameraVideoSource,
		ID_AddFileVideoDestination,
		ID_AddWindowVideoDestination,
		ID_AddRotationFilter,
		ID_ClearAllFilters,
		ID_PowerOnMachine,
		ID_PowerOffMachine,
		ID_About,
		ID_Exit
	};

	void OnPowerMachine(wxCommandEvent& event);
	void OnSetVideoSource(wxCommandEvent& event);
	void OnAddVideoDestination(wxCommandEvent& event);
	void OnAddFrameFilter(wxCommandEvent& event);
	void OnClearAllFilters(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnThreadEntering(wxThreadEvent& event);
	void OnThreadExiting(wxThreadEvent& event);
	void OnThreadError(ThreadErrorEvent& event);
	void OnThreadStatus(ThreadStatusEvent& event);
	void OnSliderChanged(wxScrollEvent& event);

	void StartThread();
	void StopThread();

	Thread* thread;
	wxSlider* slider;
	wxControl* renderControl;
};