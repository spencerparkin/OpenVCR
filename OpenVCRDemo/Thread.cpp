#include "Thread.h"
#include "Application.h"
#include "Error.h"

wxDEFINE_EVENT(EVT_THREAD_ERROR, ThreadErrorEvent);
wxDEFINE_EVENT(EVT_THREAD_STATUS, ThreadStatusEvent);
wxDEFINE_EVENT(EVT_THREAD_ENTERING, wxThreadEvent);
wxDEFINE_EVENT(EVT_THREAD_EXITING, wxThreadEvent);

ThreadErrorEvent::ThreadErrorEvent(const std::string& errorMsg) : wxThreadEvent(EVT_THREAD_ERROR)
{
	this->errorMsg = errorMsg;
}

/*virtual*/ ThreadErrorEvent::~ThreadErrorEvent()
{
}

ThreadStatusEvent::ThreadStatusEvent(const std::string& statusMsg) : wxThreadEvent(EVT_THREAD_STATUS)
{
	this->statusMsg = statusMsg;
}

/*virtual*/ ThreadStatusEvent::~ThreadStatusEvent()
{
}

Thread::Thread(wxEvtHandler* eventHandler) : wxThread(wxTHREAD_JOINABLE)
{
	this->eventHandler = eventHandler;
	this->exitSignaled = false;
}

/*virtual*/ Thread::~Thread()
{
}

/*virtual*/ wxThread::ExitCode Thread::Entry()
{
	::wxQueueEvent(this->eventHandler, new wxThreadEvent(EVT_THREAD_ENTERING));

	OpenVCR::Machine& machine = wxGetApp().machine;

	while (true)
	{
		OpenVCR::Error error;
		if (!machine.PowerOn(error))
		{
			::wxQueueEvent(this->eventHandler, new ThreadErrorEvent(error.GetErrorMessage()));
			break;
		}

		int i = 0;
		while (!this->exitSignaled)
		{
			if (!machine.Tick(error))
			{
				::wxQueueEvent(this->eventHandler, new ThreadErrorEvent(error.GetErrorMessage()));
				break;
			}

			if (i++ % 64 == 0)
			{
				std::string statusMsg;
				machine.GetStatus(statusMsg);
				::wxQueueEvent(this->eventHandler, new ThreadStatusEvent(statusMsg));
			}
		}

		if (!machine.PowerOff(error))
		{
			::wxQueueEvent(this->eventHandler, new ThreadErrorEvent(error.GetErrorMessage()));
			break;
		}

		break;
	}

	::wxQueueEvent(this->eventHandler, new wxThreadEvent(EVT_THREAD_EXITING));
	return 0;
}