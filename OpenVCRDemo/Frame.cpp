#include "Frame.h"
#include "Application.h"
#include "Thread.h"
#include <WindowVideoDestination.h>
#include <Error.h>
#include <FileVideoSource.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <inttypes.h>

Frame::Frame() : wxFrame(nullptr, wxID_ANY, "OpenVCR Demo", wxDefaultPosition, wxSize(512, 512))
{
	this->thread = nullptr;

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

	this->SetStatusBar(new wxStatusBar(this));

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
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SetFileVideoSource);
	this->Bind(wxEVT_CLOSE_WINDOW, &Frame::OnClose, this);
	this->Bind(EVT_THREAD_ENTERING, &Frame::OnThreadEntering, this);
	this->Bind(EVT_THREAD_EXITING, &Frame::OnThreadExiting, this);
	this->Bind(EVT_THREAD_STATUS, &Frame::OnThreadStatus, this);
	this->Bind(EVT_THREAD_ERROR, &Frame::OnThreadError, this);
	this->Bind(wxEVT_SCROLL_THUMBTRACK, &Frame::OnSliderChanged, this);

	this->slider = new wxSlider(this, wxID_ANY, 0, 0, 2048, wxDefaultPosition, wxSize(1024, -1));	// TODO: How do I just make it take up the full width?

	this->renderControl = new wxControl(this, wxID_ANY);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->renderControl, 1, wxALL | wxGROW, 0);
	boxSizer->Add(this->slider, 0, wxALL, 0);

	this->SetSizer(boxSizer);
}

/*virtual*/ Frame::~Frame()
{
}

void Frame::OnClose(wxCloseEvent& event)
{
	this->StopThread();
	
	wxFrame::OnCloseWindow(event);
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

void Frame::OnSliderChanged(wxScrollEvent& event)
{
	if (wxGetApp().machine.IsOn())
	{
		if (wxGetApp().machine.GetPullMethod() != OpenVCR::Machine::SourcePullMethod::SET_FRAME_POS_MANUAL)
		{
			wxGetApp().machine.SetPullMethod(OpenVCR::Machine::SourcePullMethod::SET_FRAME_POS_MANUAL);
		}

		double lerpAlpha = double(this->slider->GetValue()) / double(this->slider->GetMax());
		
		long frameCount = 0;
		OpenVCR::Error error;
		if (!wxGetApp().machine.GetVideoSource()->GetFrameCount(frameCount, error))
		{
			wxMessageBox(error.GetErrorMessage(), "Error!", wxOK | wxICON_ERROR, this);
			return;
		}
		
		double framePosition = lerpAlpha * double(frameCount - 1);
		wxGetApp().machine.SetFramePosition(framePosition);
	}
}

void Frame::OnPowerMachine(wxCommandEvent& event)
{
	OpenVCR::Error error;

	if (event.GetId() == ID_PowerOnMachine)
	{
		this->StartThread();
	}
	else if (event.GetId() == ID_PowerOffMachine)
	{
		this->StopThread();
	}
}

void Frame::StartThread()
{
	if (!this->thread)
	{
		this->thread = new Thread(this);
		this->thread->Run();
	}
}

void Frame::StopThread()
{
	if (this->thread)
	{
		this->thread->exitSignaled = true;
		this->thread->Wait();
		delete this->thread;
		this->thread = nullptr;
	}
}

void Frame::OnThreadEntering(wxThreadEvent& event)
{
}

void Frame::OnThreadExiting(wxThreadEvent& event)
{
	if (this->thread)
	{
		this->thread->Wait();
		delete this->thread;
		this->thread = nullptr;
	}
}

void Frame::OnThreadError(ThreadErrorEvent& event)
{
	wxMessageBox(event.errorMsg, "Error!", wxOK | wxICON_ERROR, this);
}

void Frame::OnThreadStatus(ThreadStatusEvent& event)	// TODO: Why is this never called?
{
	this->GetStatusBar()->SetStatusText(event.statusMsg);
}

void Frame::OnSetVideoSource(wxCommandEvent& event)
{
	OpenVCR::Error error;

	if (event.GetId() == ID_SetFileVideoSource)
	{
		wxFileDialog openFileDialog(this, "Choose Video File", "", "", "Any File (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (wxID_OK == openFileDialog.ShowModal())
		{
			auto fileVideoSource = new OpenVCR::FileVideoSource();
			fileVideoSource->SetVideoFilePath((const char*)openFileDialog.GetPath().c_str());
			wxGetApp().machine.SetVideoSource(fileVideoSource, true, error);
			if (error.GetCount() == 0)
				wxMessageBox("File video source set to: " + wxString(fileVideoSource->GetVideoFilePath().c_str()), "Message", wxOK | wxICON_INFORMATION, this);
		}
	}

	if (error.GetCount() > 0)
		wxMessageBox(error.GetErrorMessage().c_str(), "Error", wxOK | wxICON_ERROR, this);
}

void Frame::OnAddVideoDestination(wxCommandEvent& event)
{
	OpenVCR::Error error;

	if (event.GetId() == ID_AddWindowVideoDestination)
	{
		HWND windowHandle = (HWND)this->renderControl->GetHWND();
		auto windowVideoDestination = new OpenVCR::WindowVideoDestination();
		windowVideoDestination->SetWindowHandle(windowHandle);
		wxGetApp().machine.AddVideoDestination(windowVideoDestination, error);
		if (error.GetCount() == 0)
			wxMessageBox(wxString::Format("Window video destination set to HWND: %" PRIxPTR, uintptr_t(windowHandle)), "Message", wxOK | wxICON_INFORMATION, this);
	}
	else if (event.GetId() == ID_AddFileVideoDestination)
	{
		wxFileDialog saveFileDialog(this, "Choose Video File", "", "", "Any File (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (wxID_OK == saveFileDialog.ShowModal())
		{
			//...
		}
	}

	if (error.GetCount() > 0)
		wxMessageBox(error.GetErrorMessage().c_str(), "Error", wxOK | wxICON_ERROR, this);
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnAbout(wxCommandEvent& event)
{
}