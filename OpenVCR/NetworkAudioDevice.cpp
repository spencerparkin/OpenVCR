#include "NetworkAudioDevice.h"
#include "Error.h"

using namespace OpenVCR;

NetworkAudioDevice::NetworkAudioDevice(const std::string& givenName) : AudioDevice(givenName)
{
	this->sockHandle = INVALID_SOCKET;

	// This is the format we use for sending and recieving audio over the network.
	this->audioSpec.silence = 0;
	this->audioSpec.channels = 1;
	this->audioSpec.format = AUDIO_S16LSB;
	this->audioSpec.freq = 22050;
}

/*virtual*/ NetworkAudioDevice::~NetworkAudioDevice()
{
}

bool NetworkAudioDevice::GetLocalAddress(sockaddr_in& localAddress, Error& error)
{
	char hostname[128];
	int result = ::gethostname(hostname, sizeof(hostname));
	if (result == SOCKET_ERROR)
	{
		error.Add("Failed to get hostname.");
		return false;
	}

	hostent* hostEntity = ::gethostbyname(hostname);
	if (!hostEntity)
	{
		error.Add("Failed to get host entity.");
		return false;
	}

	::memset(&localAddress, 0, sizeof(sockaddr_in));
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = 0;
	for (int i = 0; hostEntity->h_addr_list[i] != nullptr; i++)
	{
		::memcpy(&localAddress.sin_addr, hostEntity->h_addr_list[i], sizeof(in_addr));
		break;		// Just take the first one we find.  But is there a better way?
	}

	return true;
}