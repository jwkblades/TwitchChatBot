#include "Message.h"
#include "PostOffice.h"

#include <cstring>
#include <utility>
#include <new>
#include <iostream>

#include <unistd.h>

MessageResponse::MessageResponse(void):
	valid(true),
	responseSet(false),
	response(NULL)
{
	// empty
}

Message::Message(void):
	mTo(),
	mFrom(),
	mResponse(NULL),
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
Message::Message(const std::string& src):
	Message(src.c_str(), src.size() + 1)
{
	// empty
}
Message::Message(const Message& src)
{
	*this = src;
}

Message& Message::operator=(const Message& src)
{
	if (this == &src)
	{
		return *this;
	}

	mResponse = src.mResponse;
	mTo = src.mTo;
	mFrom = src.mFrom;
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

const Address& Message::from(void) const
{
	return mFrom;
}
const Address& Message::to(void) const
{
	return mTo;
}

void Message::send(const Address& from, const Address& to)
{
	mFrom = from;
	mTo = to;
	PostOffice* postOffice = PostOffice::instance();
	postOffice->sendMessage(*this);
}
Message Message::sendAndAwaitResponse(const Address& from, const Address& to)
{
	mFrom = from;
	mTo = to;
	mResponse = new MessageResponse();
	PostOffice* postOffice = PostOffice::instance();
	postOffice->sendMessage(*this, true);

	for (int i = 0; i < 10; i++)
	{
		if (mResponse->responseSet && mResponse->response)
		{
			RAIIMutex responseLock(mResponse);
			Message response = *(mResponse->response);

			delete mResponse->response;
			mResponse->response = NULL;
			delete mResponse;
			mResponse = NULL;

			return response;
		}
		usleep(300);
	}
	RAIIMutex responseLock(mResponse);
	mResponse->valid = false;
	return Message();
}
void Message::respond(Message& msg)
{
	if (mResponse && mResponse->valid)
	{
		msg.mTo = mFrom;
		msg.mFrom = mTo;
		RAIIMutex responseLock(mResponse);
		mResponse->response = new Message(msg);
		mResponse->responseSet = true;
	}
	else
	{
		msg.send(mTo, mFrom);
	}
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

