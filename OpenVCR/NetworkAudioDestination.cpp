#include "NetworkAudioDestination.h"

using namespace OpenVCR;

NetworkAudioDestination::NetworkAudioDestination(const std::string& givenName) : AudioDevice(givenName)
{
}

/*virtual*/ NetworkAudioDestination::~NetworkAudioDestination()
{
}

/*static*/ NetworkAudioDestination* NetworkAudioDestination::Create(const std::string& name)
{
	return new NetworkAudioDestination(name);
}

/*virtual*/ bool NetworkAudioDestination::PowerOn(Machine* machine, Error& error)
{
	return false;
}

/*virtual*/ bool NetworkAudioDestination::PowerOff(Machine* machine, Error& error)
{
	return false;
}

/*virtual*/ bool NetworkAudioDestination::MoveData(Machine* machine, Error& error)
{
	// TODO: Can we use UDP here?

	// TODO: Like the source, I think we'll be configured to interpret the audio stream as a certain format.
	//       If the format is wrong, you just get undefined/muffled/crazy audio.
	return false;
}

/*virtual*/ bool NetworkAudioDestination::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	return false;
}