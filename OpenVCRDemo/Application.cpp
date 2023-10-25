#include "Application.h"
#include "Frame.h"

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

	return true;
}

/*virtual*/ int Application::OnExit()
{
	if (this->machine.IsOn())
		this->machine.PowerOff();

	this->machine.SetVideoSource(nullptr, true);
	this->machine.ClearAllVideoDestinations(true);

	return 0;
}