#ifndef __COMMAND_SET_H
#define __COMMAND_SET_H

#include "Command.h"

#include <map>
#include <string>

class CommandSet
{
public:
	bool registerCommand(Command cmd);
	std::vector<std::string> help(void) const;
	bool run(const std::string& name, const std::string& channel, const std::vector<std::string>& params);
private:
	std::map<std::string, Command> mCommands;
};

#endif
