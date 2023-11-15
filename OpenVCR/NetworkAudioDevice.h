#pragma once

#include "AudioDevice.h"

namespace OpenVCR
{
	class Error;

	class OPEN_VCR_API NetworkAudioDevice : public AudioDevice
	{
	public:
		NetworkAudioDevice(const std::string& givenName);
		virtual ~NetworkAudioDevice();

	protected:
		bool GetLocalAddress(sockaddr_in& localAddress, Error& error);

		SOCKET sockHandle;
	};
}