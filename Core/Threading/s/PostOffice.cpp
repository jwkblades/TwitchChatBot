#include "PostOffice.h"

PostOffice* PostOffice::oInstance = NULL;

PostOffice::PostOffice(void)
{
	// empty
}

PostOffice::~PostOffice(void)
{
	// empty
}

PostOffice* PostOffice::instance(void)
{
	RAIIMutex InstanceLock(&oInstance);
	if (!oInstance)
	{
		oInstance = new PostOffice();
	}
	return oInstance;
}

bool PostOffice::isValidInstance(const PostOffice* inst)
{
	return inst == oInstance;
}

void PostOffice::finalize(void)
{
	RAIIMutex InstanceLock(&oInstance);
	delete oInstance;
	oInstance = NULL;
}

bool PostOffice::registerAddressIfNeeded(const Address& address) const
{
	RAIIMutex MailboxLock(&mMailboxes);
	return mMailboxes.insert(std::pair<Address, std::list<Message>>(address, {})).second;
}

bool PostOffice::checkMail(const Address& self) const
{
	if (registerAddressIfNeeded(self))
	{
		return false;
	}

	std::map<Address, std::list<Message>>::const_iterator messages;
	{
		RAIIMutex MailboxLock(&mMailboxes);
		messages = mMailboxes.find(self);
	}
	RAIIMutex MessagesLock(&messages->second);
	return !messages->second.empty();
}
bool PostOffice::doesAddressExist(const Address& other) const
{
	RAIIMutex MailboxLock(&mMailboxes);
	return mMailboxes.find(other) != mMailboxes.end();
}

void PostOffice::sendMessage(Message& message, bool OOB)
{
	registerAddressIfNeeded(message.to());
	std::map<Address, std::list<Message>>::iterator messages;
	{
		RAIIMutex MailboxLock(&mMailboxes);
		messages = mMailboxes.find(message.to());
	}
	RAIIMutex MessagesLock(&messages->second);
	if (OOB)
	{
		messages->second.push_front(message);
	}
	else
	{
		messages->second.push_back(message);
	}
}

Message PostOffice::getMail(const Address& self)
{
	registerAddressIfNeeded(self);
	std::map<Address, std::list<Message>>::iterator messages;
	{
		RAIIMutex MailboxLock(&mMailboxes);
		messages = mMailboxes.find(self);
	}
	RAIIMutex MessagesLock(&messages->second);
	Message msg = messages->second.front();
	messages->second.pop_front();
	return msg;
}
