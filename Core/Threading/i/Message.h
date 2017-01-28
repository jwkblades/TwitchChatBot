#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "Address.h"
#include <cstdlib>

class Message;

struct MessageResponse
{
public:
	MessageResponse(void);
	bool valid;
	bool responseSet;
	Message* response;
};

class Message
{
public:
	Message(void);
	Message(const void* rawBuffer, std::size_t bufferSize);
	Message(const std::string& src);
	Message(const Message& src);
	~Message(void);

	std::size_t size(void) const;
	const char* raw(void) const;
	template<typename T> T get(void) const;
	void send(const Address& from, const Address& to);
	Message sendAndAwaitResponse(const Address& from, const Address& to);
	void respond(Message& msg);

	const Address& from(void) const;
	const Address& to(void) const;

	Message& operator=(const Message& src);
private:
	Address mTo;
	Address mFrom;
	MessageResponse* mResponse;
	char* mBuffer;
	std::size_t mSize;
};

#include "Message.ipp"

#endif
