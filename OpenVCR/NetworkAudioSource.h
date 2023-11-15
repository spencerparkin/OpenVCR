#pragma once

#include "NetworkAudioDevice.h"
#include "ByteStream.h"

namespace OpenVCR
{
	class OPEN_VCR_API NetworkAudioSource : public NetworkAudioDevice
	{
	public:
		NetworkAudioSource(const std::string& givenName);
		virtual ~NetworkAudioSource();

		static NetworkAudioSource* Create(const std::string& name);

		virtual bool PowerOn(Machine* machine, Error& error) override;
		virtual bool PowerOff(Machine* machine, Error& error) override;
		virtual bool MoveData(Machine* machine, Error& error) override;
		virtual bool GetSampleData(std::vector<Uint8>& sampleBuffer) override;

		void SetReceptionPort(uint16_t port);

	private:

		DWORD ThreadRun();

		static DWORD WINAPI ThreadEntryFunc(LPVOID param);

		double audioFillTimeSeconds;
		double audioGapToleranceSeconds;
		RawByteStream byteStream;
		CRITICAL_SECTION byteStreamMutex;
		uint16_t port;
		HANDLE threadHandle;
		std::vector<Uint8>* audioSampleBuffer;
	};
}