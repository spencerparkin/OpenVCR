#pragma once

#include <wx/frame.h>

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
		ID_Exit
	};

	void OnPowerMachine(wxCommandEvent& event);
	void OnSetVideoSource(wxCommandEvent& event);
	void OnAddVideoDestination(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
};