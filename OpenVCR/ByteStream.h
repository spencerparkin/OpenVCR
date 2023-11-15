#pragma once

#include "Common.h"

namespace OpenVCR
{
	// Derivatives of this class might transform the byte stream or provide a source or destination external to the stream.
	// In the latter case, bytes would only be removed or only be added to the stream.
	class OPEN_VCR_API ByteStream
	{
	public:
		ByteStream();
		virtual ~ByteStream();

		// Add the given buffer to the stream.
		virtual bool AddBytes(const uint8_t* dataBuffer, uint32_t dataBufferSize) = 0;

		// Remove a buffer up to the given size from the stream.  How bytes are removed is returned.
		virtual bool RemoveBytes(uint8_t* dataBuffer, uint32_t& dataBufferSize) = 0;

		// If applicable, return the number of bytes that can presently be removed from the stream.
		virtual uint32_t NumBytesAvailable() const = 0;
	};

	// This could be any kind of stream of data.  We don't impose any kind of interpretation of the data.
	class OPEN_VCR_API RawByteStream : public ByteStream
	{
	public:
		RawByteStream(uint32_t chunkSize = 1024);
		virtual ~RawByteStream();

		virtual bool AddBytes(const uint8_t* dataBuffer, uint32_t dataBufferSize) override;
		virtual bool RemoveBytes(uint8_t* dataBuffer, uint32_t& dataBufferSize) override;
		virtual uint32_t NumBytesAvailable() const override;

	private:
		
		struct Chunk
		{
			uint8_t* buffer;
			uint32_t bufferSize;
		};

		std::list<Chunk>* chunkList;
		
		uint32_t streamStart;
		uint32_t streamEnd;
		uint32_t chunkSize;
	};
}