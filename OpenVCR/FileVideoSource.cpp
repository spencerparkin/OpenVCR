#include "FileVideoSource.h"

using namespace OpenVCR;

FileVideoSource::FileVideoSource()
{
	this->videoFilePath = new std::string();
}

/*virtual*/ FileVideoSource::~FileVideoSource()
{
	delete this->videoFilePath;
}

void FileVideoSource::SetVideoFilePath(const std::string& videoFilePath)
{
	*this->videoFilePath = videoFilePath;
}

const std::string& FileVideoSource::GetVideoFilePath()
{
	return *this->videoFilePath;
}

/*virtual*/ bool FileVideoSource::GetFrameCount(int& frameCount)
{
	return false;
}

/*virtual*/ bool FileVideoSource::GetFrame(Frame& frame, int i)
{
	return false;
}

/*virtual*/ bool FileVideoSource::GetNextFrame(Frame& frame)
{
	return false;
}