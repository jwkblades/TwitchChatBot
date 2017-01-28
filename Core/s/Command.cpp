#include "Command.h"

Command::Command(const std::string& commandName, Command::callback_type callback):
	mCommand(commandName),
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

