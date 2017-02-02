#ifndef __UTILS_H
#define __UTILS_H

#include <vector>
#include <string>

#include "PostOffice.h"

std::vector<std::string> split(std::string str, const std::string& delimiter);

std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

struct CommandParts
{
public:
	bool valid;
	std::string command;
	std::string state;
	std::string prefix;
	std::vector<std::string> params;
};
CommandParts parseCommand(const Message& incoming);
#endif
