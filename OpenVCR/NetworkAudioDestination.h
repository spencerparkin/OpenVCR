#pragma once

#include "NetworkAudioDevice.h"

namespace OpenVCR
{
	class OPEN_VCR_API NetworkAudioDestination : public NetworkAudioDevice
	{
	public:
		NetworkAudioDestination(const std::string& givenName);
		virtual ~NetworkAudioDestination();

		static NetworkAudioDestination* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetSampleData(std::vector<Uint8>& sampleBuffer) override;

		void SetIPAddressAndPort(const std::string& ipAddr, uint16_t port);

	private:

		SDL_AudioStream* audioStream;
		sockaddr_in destinationAddress;
		std::string* ipAddr;
		uint16_t port;
	};
}