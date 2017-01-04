#include "Message.h"

#include <cstring>
#include <utility>
#include <new>
#include <iostream>

Message::Message(void):
	mBuffer(NULL),
	mSize(0)
{
	// empty
}
Message::Message(const void* rawBuffer, std::size_t bufferSize)
{
	mSize = bufferSize;
	try
	{
		mBuffer = new char[mSize];
		memcpy(mBuffer, rawBuffer, mSize);
	}
	catch(std::bad_alloc& ex)
	{
		std::cerr << "Unable to allocate memory! " << ex.what() << std::endl;
	}
}
Message::Message(const Message& src):
	Message(src.mBuffer, src.mSize)
{
	// empty
}

Message& Message::operator=(const Message& src)
{
	if (this == &src)
	{
		return *this;
	}

	mSize = src.mSize;
	try
	{
		mBuffer = new char[mSize];
		memcpy(mBuffer, src.mBuffer, mSize);
	}
	catch(std::bad_alloc& ex)
	{
		std::cerr << "Unable to allocate memory! " << ex.what() << std::endl;
	}


	return *this;
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

