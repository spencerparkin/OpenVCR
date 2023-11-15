#include "NetworkAudioSource.h"

using namespace OpenVCR;

NetworkAudioSource::NetworkAudioSource(const std::string& givenName) : AudioDevice(givenName)
{
}

/*virtual*/ NetworkAudioSource::~NetworkAudioSource()
{
}

/*static*/ NetworkAudioSource* NetworkAudioSource::Create(const std::string& name)
{
	return new NetworkAudioSource(name);
}

/*virtual*/ bool NetworkAudioSource::PowerOn(Machine* machine, Error& error)
{
	return false;
}

/*virtual*/ bool NetworkAudioSource::PowerOff(Machine* machine, Error& error)
{
	return false;
}

/*virtual*/ bool NetworkAudioSource::MoveData(Machine* machine, Error& error)
{
	// TODO: Keep track of how much time it's been since we received a packet.
	//       If the time is above a given threshold, produce silence for that
	//       amount of time, then reset the timer.  Perhaps in most cases, packets
	//       arrive quickly enough to produce one contiguous stream of sound.

	// TODO: Can we use UDP to be connectionless?
	return false;
}

/*virtual*/ bool NetworkAudioSource::GetSampleData(std::vector<Uint8>& sampleBuffer)
{
	return false;
}