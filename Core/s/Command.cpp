#include "Command.h"

Command::Command(const std::string& commandName, const std::string& desc, Command::callback_type callback):
	mCommand(commandName),
	mDescription(desc),
	mCallback(callback)
{
	// empty
}

Command::~Command(void)
{
	// empty
}

void Command::operator()(const std::string& user, const std::string& channel, const std::vector<std::string>& params) const
{
	mCallback(user, channel, params);
}

const std::string& Command::name(void) const
{
	return mCommand;
}

const std::string& Command::desc(void) const
{
	return mDescription;
}
