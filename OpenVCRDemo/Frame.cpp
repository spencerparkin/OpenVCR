#include <WinSock2.h>
#include "Frame.h"
#include "Application.h"
#include "Thread.h"
#include <FileVideoSource.h>
#include <CameraVideoSource.h>
#include <FileVideoDestination.h>
#include <WindowVideoDestination.h>
#include <FileAudioSource.h>
#include <SpeakerAudioDestination.h>
#include <FileAudioDestination.h>
#include <MicAudioSource.h>
#include <RotationFilter.h>
#include <CropFilter.h>
#include <VolumeFilter.h>
#include <Error.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/aboutdlg.h>
#include <wx/tokenzr.h>
#include <inttypes.h>

Frame::Frame() : wxFrame(nullptr, wxID_ANY, "OpenVCR Demo", wxDefaultPosition, wxSize(512, 512))
{
	this->thread = nullptr;
	this->scrubMode = false;
	this->manuallySelectDevices = false;

	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_PowerOnMachine, "Power On Machine"));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_PowerOffMachine, "Power Off Machine"));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SetupToCaptureVideo, "Setup to Capture Video"));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SetupToReplayVideo, "Setup to Replay Video"));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SetupToCaptureAudio, "Setup to Capture Audio"));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SetupToReplayAudio, "Setup to Replay Audio"));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit"));

	wxMenu* optionsMenu = new wxMenu();
	optionsMenu->Append(new wxMenuItem(optionsMenu, ID_ScrubMode, "Scrub Mode", wxEmptyString, wxITEM_CHECK));
	optionsMenu->Append(new wxMenuItem(optionsMenu, ID_ManuallySelectDevices, "Manually Select Devices", wxEmptyString, wxITEM_CHECK));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About"));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(optionsMenu, "Options");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));

	this->Bind(wxEVT_MENU, &Frame::OnSetupMachine, this, ID_SetupToCaptureVideo);
	this->Bind(wxEVT_MENU, &Frame::OnSetupMachine, this, ID_SetupToReplayVideo);
	this->Bind(wxEVT_MENU, &Frame::OnSetupMachine, this, ID_SetupToCaptureAudio);
	this->Bind(wxEVT_MENU, &Frame::OnSetupMachine, this, ID_SetupToReplayAudio);
	this->Bind(wxEVT_MENU, &Frame::OnPowerMachine, this, ID_PowerOnMachine);
	this->Bind(wxEVT_MENU, &Frame::OnPowerMachine, this, ID_PowerOffMachine);
	this->Bind(wxEVT_MENU, &Frame::OnToggleOption, this, ID_ScrubMode);
	this->Bind(wxEVT_MENU, &Frame::OnToggleOption, this, ID_ManuallySelectDevices);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_PowerOnMachine);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_PowerOffMachine);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SetupToCaptureVideo);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SetupToReplayVideo);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SetupToCaptureAudio);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SetupToReplayAudio);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ScrubMode);
	this->Bind(wxEVT_CLOSE_WINDOW, &Frame::OnClose, this);
	this->Bind(EVT_THREAD_ENTERING, &Frame::OnThreadEntering, this);
	this->Bind(EVT_THREAD_EXITING, &Frame::OnThreadExiting, this);
	this->Bind(EVT_THREAD_STATUS, &Frame::OnThreadStatus, this);
	this->Bind(EVT_THREAD_ERROR, &Frame::OnThreadError, this);
	this->Bind(wxEVT_SCROLL_THUMBTRACK, &Frame::OnSliderChanged, this);
	this->Bind(wxEVT_SIZE, &Frame::OnResize, this);

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
	
	OpenVCR::Error error;
	wxGetApp().machine.DeleteAllIODevices(error);

	wxFrame::OnCloseWindow(event);
}

void Frame::OnToggleOption(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case ID_ScrubMode:
		{
			this->scrubMode = !this->scrubMode;
			break;
		}
		case ID_ManuallySelectDevices:
		{
			this->manuallySelectDevices = !this->manuallySelectDevices;
			break;
		}
	}
	
}

void Frame::OnSetupMachine(wxCommandEvent& event)
{
	OpenVCR::Error error;

	switch (event.GetId())
	{
		case ID_SetupToCaptureVideo:
		{
			wxTextEntryDialog deviceNumberDialog(this, "Enter camera URL or device number.", "Camera", "0");
			if (wxID_OK != deviceNumberDialog.ShowModal())
				return;

			wxFileDialog saveFileDialog(this, "Choose file where video footage will get dumped.", "", "", "Video File (*.avi)|*.avi", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (wxID_OK != saveFileDialog.ShowModal())
				return;

			wxGetApp().machine.DeleteAllIODevices(error);

			auto cameraVideoSource = wxGetApp().machine.AddIODevice<OpenVCR::CameraVideoSource>("video_source", error);
			if (!cameraVideoSource)
			{
				wxMessageBox(wxString::Format("Failed to create video source: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			wxString cameraText = deviceNumberDialog.GetValue();
			long deviceNumber = 0;
			if (cameraText.ToCLong(&deviceNumber))
				cameraVideoSource->SetDeviceNumber((int)deviceNumber);
			else
				cameraVideoSource->SetCameraURL((const char*)cameraText.c_str());
			
			auto rotationFilter = wxGetApp().machine.AddIODevice<OpenVCR::RotationFilter>("rotation_filter", error);
			if (!rotationFilter)
			{
				wxMessageBox(wxString::Format("Failed to create rotation filter: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			rotationFilter->AddSourceName(cameraVideoSource->GetName());
			rotationFilter->SetRotationAngle(45.0);

			auto fileVideoDestination = wxGetApp().machine.AddIODevice<OpenVCR::FileVideoDestination>("video_destination", error);
			if (!fileVideoDestination)
			{
				wxMessageBox(wxString::Format("Failed to create video destination: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			fileVideoDestination->SetVideoFilePath((const char*)saveFileDialog.GetPath());
			fileVideoDestination->AddSourceName(rotationFilter->GetName());
			fileVideoDestination->SetFrameRate(30.0);
			fileVideoDestination->SetFrameSize(640, 480);

			auto windowVideoDestination = wxGetApp().machine.AddIODevice<OpenVCR::WindowVideoDestination>("window_destination", error);
			if (!windowVideoDestination)
			{
				wxMessageBox(wxString::Format("Failed to create window video destination: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			windowVideoDestination->SetWindowHandle(this->renderControl->GetHWND());
			windowVideoDestination->AddSourceName(rotationFilter->GetName());

			wxMessageBox("Now setup to capture and dump video file!", "Success", wxOK | wxICON_INFORMATION, this);
			break;
		}
		case ID_SetupToReplayVideo:
		{
			wxFileDialog openFileDialog(this, "Choose video file.", "", "", "Video File (*.avi)|*.avi", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (wxID_OK != openFileDialog.ShowModal())
				return;

			wxGetApp().machine.DeleteAllIODevices(error);

			auto fileVideoSource = wxGetApp().machine.AddIODevice<OpenVCR::FileVideoSource>("video_source", error);
			if (!fileVideoSource)
			{
				wxMessageBox(wxString::Format("Failed to create video source: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			fileVideoSource->SetVideoFilePath((const char*)openFileDialog.GetPath().c_str());

			auto windowVideoDestination = wxGetApp().machine.AddIODevice<OpenVCR::WindowVideoDestination>("window_destination", error);
			if (!windowVideoDestination)
			{
				wxMessageBox(wxString::Format("Failed to create window video destination: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			windowVideoDestination->SetWindowHandle(this->renderControl->GetHWND());
			windowVideoDestination->AddSourceName(fileVideoSource->GetName());

			wxMessageBox("Now setup to replay video file!", "Success", wxOK | wxICON_INFORMATION, this);
			break;
		}
		case ID_SetupToCaptureAudio:
		{
			wxFileDialog saveFileDialog(this, "Choose file where audio will get dumped.", "", "", "Wave File (*.wav)|*.wav", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (wxID_OK != saveFileDialog.ShowModal())
				return;

			wxGetApp().machine.DeleteAllIODevices(error);

			auto micAudioSource = wxGetApp().machine.AddIODevice<OpenVCR::MicAudioSource>("audio_source", error);
			if (!micAudioSource)
			{
				wxMessageBox(wxString::Format("Failed to create audio source: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			if (!this->manuallySelectDevices)
				micAudioSource->SetDeviceSubString("Logi");
			else
			{
				micAudioSource->SetDeviceSelectionCallback([=](const std::string& audioDeviceName) -> bool {
					if (wxYES == wxMessageBox(wxString::Format("Input Device: %s", audioDeviceName.c_str()), "Info", wxYES_NO, this))
						return true;
					return false;
				});
			}

			auto fileAudioDestination = wxGetApp().machine.AddIODevice<OpenVCR::FileAudioDestination>("audio_destination", error);
			if (!fileAudioDestination)
			{
				wxMessageBox(wxString::Format("Failed to create file audio destination: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			fileAudioDestination->AddSourceName(micAudioSource->GetName());
			fileAudioDestination->SetAudioFilePath((const char*)saveFileDialog.GetPath().c_str());

			wxMessageBox("Now setup to capture audio and dump to file!", "Success", wxOK | wxICON_INFORMATION, this);
			break;
		}
		case ID_SetupToReplayAudio:
		{
			wxFileDialog openFileDialog(this, "Choose audio wave file.", "", "", "Wave File (*.wav)|*.wav", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (wxID_OK != openFileDialog.ShowModal())
				return;

			wxGetApp().machine.DeleteAllIODevices(error);

			auto fileAudioSource = wxGetApp().machine.AddIODevice<OpenVCR::FileAudioSource>("audio_source", error);
			if (!fileAudioSource)
			{
				wxMessageBox(wxString::Format("Failed to create audio source: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			fileAudioSource->SetAudioFilePath((const char*)openFileDialog.GetPath().c_str());

			auto volumeFilter = wxGetApp().machine.AddIODevice<OpenVCR::VolumeFilter>("volume_control", error);
			if (!volumeFilter)
			{
				wxMessageBox(wxString::Format("Failed to create volume control: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			volumeFilter->SetVolume(1.0);
			volumeFilter->AddSourceName(fileAudioSource->GetName());

			auto speakerAudioDestination = wxGetApp().machine.AddIODevice<OpenVCR::SpeakerAudioDestination>("audio_destination", error);
			if (!speakerAudioDestination)
			{
				wxMessageBox(wxString::Format("Failed to create speaker audio destination: %s", error.GetErrorMessage().c_str()), "Error", wxOK | wxICON_ERROR, this);
				return;
			}

			speakerAudioDestination->AddSourceName(volumeFilter->GetName());

			if (!this->manuallySelectDevices)
				speakerAudioDestination->SetDeviceSubString("Logi");
			else
			{
				speakerAudioDestination->SetDeviceSelectionCallback([=](const std::string& audioDeviceName) -> bool {
					if (wxYES == wxMessageBox(wxString::Format("Output Device: %s", audioDeviceName.c_str()), "Info", wxYES_NO, this))
						return true;
					return false;
				});
			}

			fileAudioSource->SetAudioSinkName(speakerAudioDestination->GetName());

			wxMessageBox("Now setup to replay audio file!", "Success", wxOK | wxICON_INFORMATION, this);
			break;
		}
	}
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_PowerOnMachine:
		case ID_SetupToCaptureVideo:
		case ID_SetupToReplayVideo:
		case ID_SetupToCaptureAudio:
		case ID_SetupToReplayAudio:
		{
			event.Enable(!wxGetApp().machine.IsOn());
			break;
		}
		case ID_PowerOffMachine:
		{
			event.Enable(wxGetApp().machine.IsOn());
			break;
		}
		case ID_ScrubMode:
		{
			event.Check(this->scrubMode);
			break;
		}
		case ID_ManuallySelectDevices:
		{
			event.Check(this->manuallySelectDevices);
			break;
		}
	}
}

bool Frame::StringToNumberArray(const wxString& str, std::vector<int>& numberArray)
{
	wxStringTokenizer tokenizer(str, ",");
	while (tokenizer.HasMoreTokens())
	{
		wxString token = tokenizer.GetNextToken();
		long value = 0;
		if (!token.ToCLong(&value))
			return false;
		numberArray.push_back((int)value);
	}
	return true;
}

void Frame::OnSliderChanged(wxScrollEvent& event)
{
	if (wxGetApp().machine.IsOn() && this->scrubMode)
	{
		double lerpAlpha = double(this->slider->GetValue()) / double(this->slider->GetMax());
		wxGetApp().machine.SetPosition(lerpAlpha);
	}
}

void Frame::OnPowerMachine(wxCommandEvent& event)
{
	OpenVCR::Error error;

	if (event.GetId() == ID_PowerOnMachine)
	{
		if (this->scrubMode)
			wxGetApp().machine.SetDisposition(OpenVCR::Machine::Disposition::PLACE);
		else
			wxGetApp().machine.SetDisposition(OpenVCR::Machine::Disposition::PULL);

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

void Frame::OnThreadStatus(ThreadStatusEvent& event)
{
	static int count = 0, frequency = 16;
	if (count++ % frequency == 0)
		this->GetStatusBar()->SetStatusText(event.statusMsg);
}

void Frame::OnResize(wxSizeEvent& event)
{
	if (this->thread)
		this->thread->windowSizeChanged = true;

	wxFrame::OnSize(event);
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("OpenVCR Demo");
	aboutDialogInfo.SetVersion("1.0");
	aboutDialogInfo.SetDescription("This program is meant to test the OpenVCR API.");

	wxAboutBox(aboutDialogInfo);
}