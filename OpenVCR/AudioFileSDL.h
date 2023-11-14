#pragma once

#include "AudioFile.h"
#include <SDL.h>

namespace OpenVCR
{
	template<typename T, typename SampleType>
	bool AudioBufferToAudioFile(const Uint8* audioBuffer, const Uint32 audioBufferSize, const SDL_AudioSpec& audioSpec, AudioFile<T>& audioFile)
	{
		Uint32 bitsPerSample = SDL_AUDIO_BITSIZE(audioSpec.format);
		Uint32 bytesPerSample = bitsPerSample / 8;
		Uint32 bytesPerSampleFrame = bytesPerSample * audioSpec.channels;
		Uint32 totalSampleFrames = audioBufferSize / bytesPerSampleFrame;

		if (sizeof(SampleType) != bytesPerSample)
			return false;

		audioFile.setNumChannels(audioSpec.channels);
		audioFile.setNumSamplesPerChannel(totalSampleFrames);	// This is also the number of samples in a single channel.
		audioFile.setSampleRate(audioSpec.freq);
		audioFile.setBitDepth(bitsPerSample);

		const Uint8* sampleFrame = audioBuffer;
		for (int i = 0; i < (signed)totalSampleFrames; i++)
		{
			const SampleType* sampleFrameArray = reinterpret_cast<const SampleType*>(sampleFrame);
			for (int j = 0; j < audioSpec.channels; j++)
				audioFile.samples[j][i] = sampleFrameArray[j];

			sampleFrame += bytesPerSampleFrame;
		}

		return true;
	}

	template<typename T>
	bool AudioBufferFromAudiofile(Uint8*& audioBuffer, Uint32& audioBufferSize, SDL_AudioSpec& audioSpec, const AudioFile<T>& audioFile)
	{
		// TODO: Write this.  No need for now since we're using SDL_LoadWAV function.  Silly that SDL doesn't have SDL_SaveWAV function.
		audioBuffer = nullptr;
		audioBufferSize = 0;
		return false;
	}
}