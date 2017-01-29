#include "CommandSet.h"

bool CommandSet::registerCommand(Command cmd)
{
	return mCommands.insert({cmd.name(), cmd}).second;
}

std::vector<std::string> CommandSet::help(void) const
{
	std::vector<std::string> ret;
	for (const std::pair<std::string, Command>& cmd : mCommands)
	{
		ret.push_back(cmd.second.name() + ", " + cmd.second.desc());
	}
	return ret;
}

bool CommandSet::run(const std::string& name, const std::string& channel, const std::vector<std::string>& params)
{
	auto iter = mCommands.find(params.front());
	if (iter == mCommands.end())
	{
		return false;
	}
	iter->second(name, channel, params);
	return true;
}
