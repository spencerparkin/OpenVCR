#include "Frame.h"
#include "Application.h"
#include <WindowVideoDestination.h>
#include <FileVideoSource.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>

Frame::Frame() : wxFrame(nullptr, wxID_ANY, "OpenVCR Demo")
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_PowerOnMachine, "Power On Machine"));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_PowerOffMachine, "Power Off Machine"));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SetFileVideoSource, "Set File Video Source"));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SetWebCamVideoSource, "Set Web-Cam Video Source"));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SetIPCameraVideoSource, "Set IP-Camera Video Source"));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_AddFileVideoDestination, "Add File Video Destination"));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_AddWindowVideoDestination, "Add Window Video Destination"));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit"));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About"));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->Bind(wxEVT_MENU, &Frame::OnAddVideoDestination, this, ID_AddFileVideoDestination);
	this->Bind(wxEVT_MENU, &Frame::OnAddVideoDestination, this, ID_AddWindowVideoDestination);
	this->Bind(wxEVT_MENU, &Frame::OnSetVideoSource, this, ID_SetWebCamVideoSource);
	this->Bind(wxEVT_MENU, &Frame::OnSetVideoSource, this, ID_SetIPCameraVideoSource);
	this->Bind(wxEVT_MENU, &Frame::OnSetVideoSource, this, ID_SetFileVideoSource);
	this->Bind(wxEVT_MENU, &Frame::OnPowerMachine, this, ID_PowerOnMachine);
	this->Bind(wxEVT_MENU, &Frame::OnPowerMachine, this, ID_PowerOffMachine);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_PowerOnMachine);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_PowerOffMachine);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_AddFileVideoDestination);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_AddWindowVideoDestination);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SetWebCamVideoSource);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SetIPCameraVideoSource);
}

/*virtual*/ Frame::~Frame()
{
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_PowerOnMachine:
		{
			event.Enable(!wxGetApp().machine.IsOn());
			break;
		}
		case ID_PowerOffMachine:
		{
			event.Enable(wxGetApp().machine.IsOn());
			break;
		}
		case ID_AddWindowVideoDestination:
		{
			auto windowDestinationFound = []() -> bool {
				for (int i = 0; i < wxGetApp().machine.GetNumVideoDestination(); i++)
				{
					OpenVCR::VideoDestination* videoDestination = wxGetApp().machine.GetVideoDestination(i);
					if (dynamic_cast<OpenVCR::WindowVideoDestination*>(videoDestination))
						return true;
				}
				return false;
			};

			event.Enable(!wxGetApp().machine.IsOn() && !windowDestinationFound());
			break;
		}
		case ID_AddFileVideoDestination:
		case ID_SetWebCamVideoSource:
		case ID_SetIPCameraVideoSource:
		case ID_SetFileVideoSource:
		{
			event.Enable(!wxGetApp().machine.IsOn());
			break;
		}
	}
}

void Frame::OnPowerMachine(wxCommandEvent& event)
{
	if (event.GetId() == ID_PowerOnMachine)
	{
		if (!wxGetApp().machine.PowerOn())
		{
			wxMessageBox("Failed to power on machine!", "Error", wxOK | wxICON_ERROR, this);
			wxGetApp().machine.PowerOff();
		}
	}
	else if (event.GetId() == ID_PowerOffMachine)
	{
		if (!wxGetApp().machine.PowerOff())
		{
			wxMessageBox("Failed to power off machine!", "Error", wxOK | wxICON_ERROR, this);
		}
	}
}

void Frame::OnSetVideoSource(wxCommandEvent& event)
{
	if (event.GetId() == ID_SetFileVideoSource)
	{
		wxFileDialog openFileDialog(this, "Choose Video File", "", "", "Any File (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (wxID_OK == openFileDialog.ShowModal())
		{
			auto fileVideoSource = new OpenVCR::FileVideoSource();
			fileVideoSource->SetVideoFilePath((const char*)openFileDialog.GetPath().c_str());
			wxGetApp().machine.SetVideoSource(fileVideoSource, true);
			wxMessageBox("File video source set to: " + wxString(fileVideoSource->GetVideoFilePath().c_str()), "Message", wxOK | wxICON_INFORMATION, this);
		}
	}
}

void Frame::OnAddVideoDestination(wxCommandEvent& event)
{
	if (event.GetId() == ID_AddWindowVideoDestination)
	{
		HANDLE windowHandle = (HANDLE)this->GetHWND();
		auto windowVideoDestination = new OpenVCR::WindowVideoDestination();
		windowVideoDestination->SetWindowHandle(windowHandle);
		wxGetApp().machine.AddVideoDestination(windowVideoDestination);
		wxMessageBox(wxString::Format("Window video destination set to HWND: 0x%08x", uintptr_t(windowHandle)), "Message", wxOK | wxICON_INFORMATION, this);
	}
	else if (event.GetId() == ID_AddFileVideoDestination)
	{
		wxFileDialog saveFileDialog(this, "Choose Video File", "", "", "Any File (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (wxID_OK == saveFileDialog.ShowModal())
		{
			//...
		}
	}
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnAbout(wxCommandEvent& event)
{
}