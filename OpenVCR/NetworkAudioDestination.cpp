#include "NetworkAudioDestination.h"
#include "Machine.h"
#include "Error.h"

using namespace OpenVCR;

NetworkAudioDestination::NetworkAudioDestination(const std::string& givenName) : NetworkAudioDevice(givenName)
{
	this->audioStream = nullptr;
	this->ipAddr = new std::string();
	this->port = 0;
	::memset(&this->destinationAddress, 0, sizeof(sockaddr_in));
}

/*virtual*/ NetworkAudioDestination::~NetworkAudioDestination()
{
	delete this->ipAddr;
}

/*static*/ NetworkAudioDestination* NetworkAudioDestination::Create(const std::string& name)
{
	return new NetworkAudioDestination(name);
}

/*virtual*/ bool NetworkAudioDestination::PowerOn(Machine* machine, Error& error)
{
	if (this->sockHandle != INVALID_SOCKET)
	{
		error.Add("Socket already setup!");
		return false;
	}

	if (this->ipAddr->length() == 0 || this->port == 0)
	{
		error.Add("No IP address and port given.");
		return false;
	}

	if (this->GetNumSourceNames() != 1)
	{
		error.Add("Network audio destination expected exactly one source.");
		return false;
	}

	AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(this->GetSourceName(0));
	if (!audioDevice)
	{
		error.Add("Network audio destination failed to find its source.");
		return false;
	}

	if (this->audioStream)
	{
		error.Add("Audio stream already setup!");
		return false;
	}

	SDL_AudioSpec* sourceSpec = audioDevice->GetAudioSpec();
	this->audioStream = SDL_NewAudioStream(
		sourceSpec->format,
		sourceSpec->channels,
		sourceSpec->freq,
		this->audioSpec.format,
		this->audioSpec.channels,
		this->audioSpec.freq);
	if (!this->audioStream)
	{
		error.Add("Failed to create audio stream!");
		return false;
	}

	WSAData wsaData;
	int result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		error.Add(std::format("WSA start-up failed with error code: {}", result));
		return false;
	}

	::memset(&this->destinationAddress, 0, sizeof(sockaddr_in));
	this->destinationAddress.sin_family = AF_INET;
	this->destinationAddress.sin_addr.S_un.S_addr = ::inet_addr(this->ipAddr->c_str());
	this->destinationAddress.sin_port = ::htons(this->port);

	// Note that according to the docs, we don't need to explicitly bind our socket to the local address.
	// An implicit bind will occur on the first call to the sendto() function.
	this->sockHandle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockHandle == INVALID_SOCKET)
	{
		error.Add(std::format("Failed to create socket with error code: {}", ::WSAGetLastError()));
		return false;
	}

	this->poweredOn = true;
	return true;
}

/*virtual*/ bool NetworkAudioDestination::PowerOff(Machine* machine, Error& error)
{
	if (this->sockHandle != INVALID_SOCKET)
	{
		::closesocket(this->sockHandle);
		this->sockHandle = INVALID_SOCKET;
	}

	if (this->audioStream)
	{
		SDL_FreeAudioStream(this->audioStream);
		this->audioStream = nullptr;
	}

	::WSACleanup();
	this->poweredOn = false;
	return true;
}

/*virtual*/ bool NetworkAudioDestination::MoveData(Machine* machine, Error& error)
{
	AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(this->GetSourceName(0));
	if (!audioDevice)
	{
		error.Add("Network audio destination failed to find its source.");
		return false;
	}

	if (!audioDevice->IsComplete())
		return true;

	// Our source is ready.  Grab some audio, if any available.
	std::vector<Uint8> sampleBuffer;
	if (audioDevice->GetSampleData(sampleBuffer))
	{
		if (0 != SDL_AudioStreamPut(this->audioStream, sampleBuffer.data(), (int)sampleBuffer.size()))
		{
			error.Add("Failed to put audio sample to audio stream.");
			return false;
		}
	}

	// Even if we didn't put anything to the stream, we may sometimes have something to get from the stream.
	Uint32 numBytesAvailable = SDL_AudioStreamAvailable(this->audioStream);
	if (numBytesAvailable > 0)
	{
		sampleBuffer.resize(numBytesAvailable);
		Uint32 numBytesGotten = SDL_AudioStreamGet(this->audioStream, sampleBuffer.data(), numBytesAvailable);
		if (numBytesGotten == -1)
		{
			error.Add("Failed to get audio sample from audio stream.");
			return false;
		}

		if (numBytesGotten != numBytesAvailable)
		{
			error.Add("Did not get all bytes available.");
			return false;
		}

		// Should a time stamp be embedded in the datagram?  For now, we always assume all datagrams are contiguous in time.
		int numBytesSent = ::sendto(this->sockHandle, (const char*)sampleBuffer.data(), numBytesGotten, 0, (const sockaddr*)&this->destinationAddress, sizeof(sockaddr_in));
		if (numBytesSent == SOCKET_ERROR)
		{
			error.Add(std::format("Failed to send audio sample along socket to the destination address.  Error code: {}", ::WSAGetLastError()));
			return false;
		}
	}

	this->complete = true;
	return true;
}

/*virtual*/ bool NetworkAudioDestination::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	return false;
}

void NetworkAudioDestination::SetIPAddressAndPort(const std::string& ipAddr, uint16_t port)
{
	*this->ipAddr = ipAddr;
	this->port = port;
}