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
	// TODO: Can we use UDP here?  The first bit of data sent should probably be
	//       a structure indicating the format of the subsequent audio stream.
	return false;
}

/*virtual*/ bool NetworkAudioDestination::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	return false;
}