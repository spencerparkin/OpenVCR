#include <WinSock2.h>
#include "Application.h"
#include "Frame.h"
#include "Machine.h"
#include <wx/msgdlg.h>

wxIMPLEMENT_APP(Application);

Application::Application()
{
	this->frame = nullptr;
}

/*virtual*/ Application::~Application()
{
}

/*virtual*/ bool Application::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new Frame();
	this->frame->Show();

	OpenVCR::Error error;
	if (!OpenVCR::Machine::Setup(error))
	{
		wxMessageBox(error.GetErrorMessage(), "Error", wxOK | wxICON_ERROR, this->frame);
		return false;
	}

	return true;
}

/*virtual*/ int Application::OnExit()
{
	OpenVCR::Error error;
	OpenVCR::Machine::Shutdown(error);

	return 0;
}