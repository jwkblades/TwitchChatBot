#include "utils.h"

std::vector<std::string> split(std::string str, const std::string& delimiter)
{
	std::vector<std::string> parts;

	while (!str.empty())
	{
		std::size_t pos = str.find(delimiter);
		std::string part = str.substr(0, pos);
		if (pos == std::string::npos)
		{
			str = "";
		}
		else
		{
			str = str.substr(pos + delimiter.size());
		}

		if (!part.empty())
		{
			parts.push_back(part);
		}
	}

	return parts;
}

std::string join(const std::vector<std::string>& parts, const std::string& delimiter)
{
	std::string ret = "";
	for (std::vector<std::string>::const_iterator iter = parts.begin(); iter != parts.end(); iter++)
	{
		ret += *iter;
		auto tmpIter = iter;
		tmpIter++;
		if (tmpIter != parts.end())
		{
			ret += delimiter;
		}
	}
	return ret;
}

CommandParts parseCommand(const Message& incoming)
{
	std::string command = std::string(incoming.raw(), incoming.size() - 1);
	std::string prefix = "";
	std::vector<std::string> params;

	std::vector<std::string> parts = split(command, " ");
	if (parts.empty() || parts.at(0).empty()) // we have an empty prefix/ command...
	{
		return {false, {}, {}, {}};
	}

	if(parts.at(0).at(0) == ':')
	{
		// We have a prefix, meaning that the second item (if it exists) is the command.
		prefix = parts.at(0);
		parts.erase(parts.begin());
	}

	if (parts.empty())
	{
		return {false, {}, {}, {}};
	}

	command = parts.at(0);
	parts.erase(parts.begin());

	while (!parts.empty())
	{
		if (parts.at(0).size() > 0 && parts.at(0).at(0) == ':')
		{
			parts.at(0) = parts.at(0).substr(1);
			params.push_back(join(parts, " "));
			parts.clear();
		}
		else
		{
			params.push_back(parts.at(0));
			parts.erase(parts.begin());
		}
	}
	return {
		true,
		command,
		prefix,
		params
	};
}
