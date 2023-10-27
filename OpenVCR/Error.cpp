#include "Error.h"

using namespace OpenVCR;

Error::Error()
{
	this->errorArray = new std::vector<std::string>();
}

/*virtual*/ Error::~Error()
{
	delete this->errorArray;
}

int Error::GetCount() const
{
	return (int)this->errorArray->size();
}

void Error::Add(const std::string& errorMsg)
{
	this->errorArray->push_back(errorMsg);
}

std::string Error::GetErrorMessage() const
{
	std::string errorMsg;
	for (const std::string& error : *this->errorArray)
		errorMsg += error + "\n";
	return errorMsg;
}

void Error::Clear()
{
	this->errorArray->clear();
}