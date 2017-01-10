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

void sendMessage(const Address& addr, const std::string& str)
{
	PostOffice* postOffice = PostOffice::instance();

	Message msg(str.c_str(), str.size() + 1);
	postOffice->sendMessage(addr, msg);
}
