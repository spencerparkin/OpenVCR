#include "FileAudioDestination.h"
#include "Error.h"
#include "Machine.h"
#include "AudioFile.h"

using namespace OpenVCR;

FileAudioDestination::FileAudioDestination(const std::string& givenName) : AudioDevice(givenName)
{
	this->audioFilePath = new std::string();
	this->audioStream = nullptr;

	::memset(&this->inputSpec, 0, sizeof(SDL_AudioSpec));

	// Dumping uncompressed audio to disk takes up a lot of space, so choose
	// an output format that is as minimal as possible.
	this->outputSpec.size = 0;
	this->outputSpec.padding = 0;
	this->outputSpec.samples = 4096;
	this->outputSpec.silence = 0;
	this->outputSpec.channels = 1;
	this->outputSpec.format = AUDIO_S16LSB;		// What about AUDIO_S8, or is that really poor quality?
	this->outputSpec.freq = 22050;
	this->outputSpec.callback = nullptr;
	this->outputSpec.userdata = nullptr;
}

/*virtual*/ FileAudioDestination::~FileAudioDestination()
{
	delete this->audioFilePath;
}

/*static*/ FileAudioDestination* FileAudioDestination::Create(const std::string& name)
{
	return new FileAudioDestination(name);
}

/*virtual*/ bool FileAudioDestination::PowerOn(Machine* machine, Error& error)
{
	if (this->audioStream)
	{
		error.Add("Audio stream already non-null!");
		return false;
	}

	AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(*this->sourceName);
	if (!audioDevice)
	{
		error.Add("File audio destination has no source device.");
		return false;
	}

	memcpy(&this->inputSpec, audioDevice->GetAudioSpec(), sizeof(SDL_AudioSpec));
	
	this->audioStream = SDL_NewAudioStream(
		this->inputSpec.format,
		this->inputSpec.channels,
		this->inputSpec.freq,
		this->outputSpec.format,
		this->outputSpec.channels,
		this->outputSpec.freq);
	if (!this->audioStream)
	{
		error.Add(std::format("Failed to create audio stream: {}", SDL_GetError()));
		return false;
	}

	return true;
}

/*virtual*/ bool FileAudioDestination::PowerOff(Machine* machine, Error& error)
{
	// TODO: We may need to flush the audio to disk as we capture it so that we don't fill up RAM.
	if (this->audioStream)
	{
		SDL_AudioStreamFlush(this->audioStream);

		Uint32 audioBufferSize = SDL_AudioStreamAvailable(this->audioStream);
		if (audioBufferSize > 0)
		{
			Uint8* audioBuffer = new Uint8[audioBufferSize];
			if (!audioBuffer)
				error.Add("Could not allocate memory for audio buffer!");
			else
			{
				int result = SDL_AudioStreamGet(this->audioStream, audioBuffer, audioBufferSize);
				if (result == 0)
					error.Add(std::string("Failed to get samples from stream: {}", SDL_GetError()));
				else
				{
					AudioFile<int> audioFile;
					audioFile.setNumChannels(this->outputSpec.channels);
					audioFile.setNumSamplesPerChannel(audioBufferSize / 2);
					audioFile.setSampleRate(this->outputSpec.freq);
					audioFile.setBitDepth(16);

					Sint16* audioBufferSamples = reinterpret_cast<Sint16*>(audioBuffer);
					for (int i = 0; i < (signed)audioBufferSize / 2; i++)
						audioFile.samples[0][i] = audioBufferSamples[i];

					delete[] audioBuffer;

					bool saved = audioFile.save(*this->audioFilePath, AudioFileFormat::Wave);
					if (!saved)
						error.Add(std::format("Failed to save file: {}", this->audioFilePath->c_str()));
				}
			}
		}

		SDL_FreeAudioStream(this->audioStream);
	}

	return error.GetCount() == 0;
}

/*virtual*/ bool FileAudioDestination::MoveData(Machine* machine, Error& error)
{
	AudioDevice* audioDevice = machine->FindIODevice<AudioDevice>(*this->sourceName);
	if (!audioDevice)
	{
		error.Add("File audio destination has no source device.");
		return false;
	}

	if (!audioDevice->IsComplete())
		return true;

	std::vector<Uint8> sampleBuffer;
	if (audioDevice->GetSampleData(sampleBuffer))
	{
		int result = SDL_AudioStreamPut(this->audioStream, sampleBuffer.data(), (int)sampleBuffer.size());
		if (result < 0)
		{
			error.Add(std::string("Failed to put samples into audio stream: {}", SDL_GetError()));
			return false;
		}
	}

	this->complete = true;
	return true;
}

/*virtual*/ int FileAudioDestination::GetSortKey() const
{
	return 1;
}

void FileAudioDestination::SetAudioFilePath(const std::string& audioFilePath)
{
	*this->audioFilePath = audioFilePath;
}

const std::string& FileAudioDestination::GetAudioFilePath() const
{
	return *this->audioFilePath;
}