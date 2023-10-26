#pragma once

#include <wx/thread.h>
#include <wx/event.h>

class ThreadErrorEvent : public wxThreadEvent
{
public:
	ThreadErrorEvent(const std::string& errorMsg);
	virtual ~ThreadErrorEvent();

	wxString errorMsg;
};

class ThreadStatusEvent : public wxThreadEvent
{
public:
	ThreadStatusEvent(const std::string& statusMsg);
	virtual ~ThreadStatusEvent();

	wxString statusMsg;
};

wxDECLARE_EVENT(EVT_THREAD_ERROR, ThreadErrorEvent);
wxDECLARE_EVENT(EVT_THREAD_STATUS, ThreadStatusEvent);
wxDECLARE_EVENT(EVT_THREAD_ENTERING, wxThreadEvent);
wxDECLARE_EVENT(EVT_THREAD_EXITING, wxThreadEvent);

class Thread : public wxThread
{
public:
	Thread(wxEvtHandler* eventHandler);
	virtual ~Thread();

	virtual ExitCode Entry() override;

	bool exitSignaled;
	wxEvtHandler* eventHandler;
};