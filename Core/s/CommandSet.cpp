#include "CommandSet.h"

#include <iostream>
using namespace std;

bool CommandSet::registerCommand(Command cmd)
{
	cout << "=== Registering command: '" << cmd.name() << "'" << endl;
	return mCommands.insert({cmd.name(), cmd}).second;
}

bool CommandSet::run(const std::string& name, const std::string& channel, const std::vector<std::string>& params)
{
	cout << "=== Attempting to find command '" << params.front() << "' to run user=" << name << ", channel=" << channel << "." << endl;
	auto iter = mCommands.find(params.front());
	if (iter == mCommands.end())
	{
		return false;
	}
	iter->second(name, channel, params);
	return true;
}
