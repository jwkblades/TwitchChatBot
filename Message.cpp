#include "Message.h"

#include <cstring>
#include <utility>

Message::Message(void):
	mBuffer(NULL),
	mSize(0)
{
	// empty
}
Message::Message(const void* rawBuffer, std::size_t bufferSize)
{
	mSize = bufferSize;
	mBuffer = new char[mSize];
	memcpy(mBuffer, rawBuffer, mSize);
}
Message::Message(const Message& src):
	Message(src.mBuffer, src.mSize)
{
	// empty
}

Message::~Message(void)
{
	delete [] mBuffer;
	mBuffer = NULL;
}

std::size_t Message::size(void) const
{
	return mSize;
}

const char* Message::raw(void) const
{
	return mBuffer;
}

