#include "FileAudioSource.h"
#include "Error.h"
#include "Machine.h"

using namespace OpenVCR;

FileAudioSource::FileAudioSource(const std::string& givenName) : AudioDevice(givenName)
{
	this->audioFilePath = new std::string();
	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;
}

/*virtual*/ FileAudioSource::~FileAudioSource()
{
	delete this->audioFilePath;
}

/*static*/ FileAudioSource* FileAudioSource::Create(const std::string& name)
{
	return new FileAudioSource(name);
}

/*virtual*/ bool FileAudioSource::PowerOn(Machine* machine, Error& error)
{
	if (!SDL_LoadWAV(this->audioFilePath->c_str(), &this->audioSpec, &this->audioBuffer, &this->audioBufferSize))
	{
		error.Add(SDL_GetError());
		return false;
	}

	return true;
}

/*virtual*/ bool FileAudioSource::PowerOff(Machine* machine, Error& error)
{
	SDL_FreeWAV(this->audioBuffer);

	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;

	return true;
}

/*virtual*/ bool FileAudioSource::MoveData(Machine* machine, Error& error)
{
	// I had this dream of filtering and mixing a stream of audio data in real-time, but I'm an
	// idiot, and I have no idea how to do that.  Instead, everything is always ready, from
	// beginning to end, right up front from the get-go.
	this->complete = true;
	return true;
}

/*virtual*/ Uint8* FileAudioSource::GetSampleData(Uint32& size)
{
	size = this->audioBufferSize;
	return this->audioBuffer;
}

void FileAudioSource::SetAudioFilePath(const std::string& audioFilePath)
{
	*this->audioFilePath = audioFilePath;
}

const std::string& FileAudioSource::GetAudioFilePath() const
{
	return *this->audioFilePath;
}