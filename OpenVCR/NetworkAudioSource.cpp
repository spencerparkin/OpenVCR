#include "NetworkAudioSource.h"
#include "Machine.h"
#include "Error.h"

using namespace OpenVCR;

NetworkAudioSource::NetworkAudioSource(const std::string& givenName) : NetworkAudioDevice(givenName)
{
	this->port = 0;
	this->threadHandle = NULL;
	this->audioFillTimeSeconds = 0.0;
	this->audioGapToleranceSeconds = 1.0;
	this->audioSampleBuffer = new std::vector<Uint8>();
}

/*virtual*/ NetworkAudioSource::~NetworkAudioSource()
{
	delete this->audioSampleBuffer;
}

/*static*/ NetworkAudioSource* NetworkAudioSource::Create(const std::string& name)
{
	return new NetworkAudioSource(name);
}

/*virtual*/ bool NetworkAudioSource::PowerOn(Machine* machine, Error& error)
{
	::InitializeCriticalSection(&this->byteStreamMutex);

	if (this->sockHandle != INVALID_SOCKET)
	{
		error.Add("Socket already setup!");
		return false;
	}

	if (this->port == 0)
	{
		error.Add("No port given.");
		return false;
	}

	if (this->GetNumSourceNames() == 0)
	{
		error.Add("Network audio source doesn't take any sources.  It has no dependencies.");
		return false;
	}

	WSAData wsaData;
	int result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		error.Add(std::format("WSA start-up failed with error code: {}", result));
		return false;
	}

	sockaddr_in localAddress;
	if (!this->GetLocalAddress(localAddress, error))
		return false;

	localAddress.sin_port = this->port;

	this->sockHandle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockHandle == INVALID_SOCKET)
	{
		error.Add(std::format("Failed to create socket with error code: {}", ::WSAGetLastError()));
		return false;
	}

	result = ::bind(this->sockHandle, (sockaddr*)&localAddress, sizeof(localAddress));
	if (result == SOCKET_ERROR)
	{
		error.Add(std::format("Failed to bind socket with error code: {}", ::WSAGetLastError()));
		return false;
	}

	if (this->threadHandle != NULL)
	{
		error.Add("Thread already setup!");
		return false;
	}

	this->threadHandle = ::CreateThread(NULL, 0, &NetworkAudioSource::ThreadEntryFunc, this, 0, NULL);
	if (this->threadHandle == NULL)
	{
		error.Add(std::format("Failed to create thread!  Error code: {}", ::GetLastError()));
		return false;
	}

	this->audioFillTimeSeconds = double(::clock()) / double(CLOCKS_PER_SEC);

	this->poweredOn = true;
	return true;
}

/*virtual*/ bool NetworkAudioSource::PowerOff(Machine* machine, Error& error)
{
	if (this->sockHandle != INVALID_SOCKET)
	{
		::closesocket(this->sockHandle);
		this->sockHandle = INVALID_SOCKET;
	}

	// Closing the socket will have signaled our thread to exit.  Join it now.
	if (this->threadHandle != NULL)
	{
		::WaitForSingleObject(this->threadHandle, INFINITE);
		this->threadHandle = NULL;
	}

	::WSACleanup();

	::DeleteCriticalSection(&this->byteStreamMutex);

	this->poweredOn = false;
	return true;
}

/*virtual*/ bool NetworkAudioSource::MoveData(Machine* machine, Error& error)
{
	double presentTimeSeconds = double(::clock()) / double(CLOCKS_PER_SEC);
	double elapsedTimeSeconds = presentTimeSeconds - this->audioFillTimeSeconds;

	// We must provide audio data to fill the entire elapsed time.  Do we have anything from the socket?
	::EnterCriticalSection(&this->byteStreamMutex);
	Uint32 numBytesAvailable = this->byteStream.NumBytesAvailable();
	::LeaveCriticalSection(&this->byteStreamMutex);

	// How much time can we fill using what we have from the socket?
	double streamTimeSeconds = this->AudioBufferOffsetToTimeSeconds(numBytesAvailable);

	// There are now two cases to deal with.
	if (streamTimeSeconds >= elapsedTimeSeconds)
	{
		// Consume just enough from the stream to fill up to the present time.
		Uint32 numBytes = this->AudioBufferOffsetFromTimeSeconds(elapsedTimeSeconds);
		assert(numBytes <= numBytesAvailable);
		this->audioSampleBuffer->resize(numBytes);
		Uint32 numBytesRemoved = numBytes;
		::EnterCriticalSection(&this->byteStreamMutex);
		this->byteStream.RemoveBytes(this->audioSampleBuffer->data(), numBytesRemoved);
		::LeaveCriticalSection(&this->byteStreamMutex);
		assert(numBytesRemoved == numBytes);
		this->audioFillTimeSeconds = presentTimeSeconds;
	}
	else
	{
		// Consume the entire stream.
		Uint32 numBytesRemoved = numBytesAvailable;
		this->audioSampleBuffer->resize(numBytesAvailable);
		::EnterCriticalSection(&this->byteStreamMutex);
		this->byteStream.RemoveBytes(this->audioSampleBuffer->data(), numBytesRemoved);
		::LeaveCriticalSection(&this->byteStreamMutex);
		assert(numBytesRemoved == numBytesAvailable);

		// At this point we have to decide whether we just hope the stream can catch up to the present,
		// or we just fill the remainder of the time with silence.  Use a gap-tolerance to decide.
		if (elapsedTimeSeconds - streamTimeSeconds < this->audioGapToleranceSeconds)
			this->audioFillTimeSeconds += streamTimeSeconds;
		else
		{
			this->audioFillTimeSeconds = presentTimeSeconds;
			Uint32 numBytes = this->AudioBufferOffsetFromTimeSeconds(elapsedTimeSeconds - streamTimeSeconds);
			for (Uint32 i = 0; i < numBytes; i++)
				this->audioSampleBuffer->push_back(this->audioSpec.silence);
		}
	}

	this->complete = true;
	return true;
}

/*virtual*/ bool NetworkAudioSource::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	sampleBuffer.resize(this->audioSampleBuffer->size());
	::memcpy(sampleBuffer.data(), this->audioSampleBuffer->data(), this->audioSampleBuffer->size());
	return sampleBuffer.size() > 0;
}

void NetworkAudioSource::SetReceptionPort(uint16_t port)
{
	this->port = port;
}

DWORD NetworkAudioSource::ThreadRun()
{
	while (true)
	{
		Uint8 audioBuffer[1024];

		// Block here waiting for data on the socket.
		int numBytesRecieved = ::recvfrom(this->sockHandle, (char*)audioBuffer, sizeof(audioBuffer), 0, nullptr, nullptr);
		
		// If an error ocurrs or the socket is closed gracefully, we exit the thread.
		if (numBytesRecieved == SOCKET_ERROR || numBytesRecieved == 0)
			break;

		// Send off the audio data to the main thread as quickly as possible.
		::EnterCriticalSection(&this->byteStreamMutex);
		this->byteStream.AddBytes(audioBuffer, numBytesRecieved);
		::LeaveCriticalSection(&this->byteStreamMutex);
	}

	return 0;
}

/*static*/ DWORD WINAPI NetworkAudioSource::ThreadEntryFunc(LPVOID param)
{
	auto networkAudioSource = static_cast<NetworkAudioSource*>(param);
	return networkAudioSource->ThreadRun();
}