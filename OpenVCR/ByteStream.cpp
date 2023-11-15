#include "ByteStream.h"

using namespace OpenVCR;

//----------------------------- ByteStream -----------------------------

ByteStream::ByteStream()
{
}

/*virtual*/ ByteStream::~ByteStream()
{
}

//----------------------------- RawByteStream -----------------------------

RawByteStream::RawByteStream(uint32_t chunkSize /*= 1024*/)
{
	this->streamStart = 0;
	this->streamEnd = 0;
	this->chunkSize = chunkSize;
	this->chunkList = new std::list<Chunk>();
}

/*virtual*/ RawByteStream::~RawByteStream()
{
	for (Chunk& chunk : *this->chunkList)
		delete[] chunk.buffer;

	delete this->chunkList;
}

/*virtual*/ bool RawByteStream::AddBytes(const uint8_t* dataBuffer, uint32_t dataBufferSize)
{
	if (this->chunkList->size() == 0)
	{
		this->chunkList->push_back(Chunk{ new uint8_t[this->chunkSize], this->chunkSize });
		this->streamStart = 0;
		this->streamEnd = 0;
	}

	Chunk* chunk = &this->chunkList->back();

	for (uint32_t i = 0; i < dataBufferSize; i++)
	{
		chunk->buffer[this->streamEnd++] = dataBuffer[i];
		
		if (this->streamEnd == chunk->bufferSize)
		{
			this->streamEnd = 0;
			this->chunkList->push_back(Chunk{ new uint8_t[this->chunkSize], this->chunkSize });
			chunk = &this->chunkList->back();
		}
	}

	return dataBufferSize > 0;
}

/*virtual*/ bool RawByteStream::RemoveBytes(uint8_t* dataBuffer, uint32_t& dataBufferSize)
{
	uint32_t numBytesRemoved = 0;

	if (this->chunkList->size() != 0 && !(this->chunkList->size() == 1 && this->streamStart == this->streamEnd))
	{
		Chunk* chunk = &*this->chunkList->begin();

		for (uint32_t i = 0; i < dataBufferSize; i++)
		{
			dataBuffer[i] = chunk->buffer[this->streamStart++];

			if (this->chunkList->size() == 1 && this->streamStart == this->streamEnd)
				break;

			if (this->streamStart == chunk->bufferSize)
			{
				this->streamStart = 0;
				delete[] chunk->buffer;
				this->chunkList->erase(this->chunkList->begin());
				chunk = &*this->chunkList->begin();
			}
		}
	}

	dataBufferSize = numBytesRemoved;
	return numBytesRemoved > 0;
}

/*virtual*/ uint32_t RawByteStream::NumBytesAvailable() const
{
	uint32_t byteCount = 0;

	if (this->chunkList->size() > 0)
	{
		if (this->chunkList->size() == 1)
			byteCount = this->streamEnd - this->streamStart;
		else
		{
			uint32_t i = 0;
			for (Chunk& chunk : *this->chunkList)
			{
				if (i == 0)
					byteCount += chunk.bufferSize - this->streamStart;
				else if (i == this->chunkList->size() - 1)
					byteCount += this->streamEnd;
				else
					byteCount += chunk.bufferSize;

				i++;
			}
		}
	}

	return byteCount;
}