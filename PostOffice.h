#ifndef __POST_OFFICE_H
#define __POST_OFFICE_H

#include <list>
#include <map>

#include "Address.h"
#include "Message.h"
#include "RAIIMutex.h"

class PostOffice
{
public:
	static PostOffice* instance(void);
	static bool isValidInstance(const PostOffice* inst);

	static void finalize(void);

	bool registerAddress(const Address& address);

	bool checkMail(const Address& self) const;
	bool isValidAddress(const Address& other) const;

	void sendMessage(const Address& to, Message& message);
	Message getMail(const Address& self);

private:
	PostOffice(void);
	~PostOffice(void);

	std::map<Address, std::list<Message>> mMailboxes;

	static PostOffice* oInstance;
};

#endif
