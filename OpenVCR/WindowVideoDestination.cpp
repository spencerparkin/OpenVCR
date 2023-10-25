#include "WindowVideoDestination.h"

using namespace OpenVCR;

WindowVideoDestination::WindowVideoDestination()
{
	this->windowHandle = INVALID_HANDLE_VALUE;
}

/*virtual*/ WindowVideoDestination::~WindowVideoDestination()
{
}

void WindowVideoDestination::SetWindowHandle(HANDLE windowHandle)
{
	this->windowHandle = windowHandle;
}

HANDLE WindowVideoDestination::GetWindowHandle()
{
	return this->windowHandle;
}

/*virtual*/ bool WindowVideoDestination::AddFrame(Frame& frame)
{
	return false;
}