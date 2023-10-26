#pragma once

#include <wx/frame.h>
#include <wx/timer.h>

class Frame : public wxFrame
{
public:
	Frame();
	virtual ~Frame();

	enum
	{
		ID_SetFileVideoSource = wxID_HIGHEST,
		ID_SetIPCameraVideoSource,
		ID_SetWebCamVideoSource,
		ID_AddFileVideoDestination,
		ID_AddWindowVideoDestination,
		ID_PowerOnMachine,
		ID_PowerOffMachine,
		ID_About,
		ID_Timer,
		ID_Exit
	};

	void OnPowerMachine(wxCommandEvent& event);
	void OnSetVideoSource(wxCommandEvent& event);
	void OnAddVideoDestination(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnTimer(wxTimerEvent& event);

	wxTimer timer;
	bool inTimer;
};