#pragma once

#include <wx/setup.h>
#include <wx/app.h>
#include <Machine.h>

class Frame;

class Application : public wxApp
{
public:
	Application();
	virtual ~Application();

	virtual bool OnInit() override;
	virtual int OnExit() override;

	Frame* frame;

	OpenVCR::Machine machine;
};

wxDECLARE_APP(Application);