#ifndef __COMMAND_H
#define __COMMAND_H

#include <functional>
#include <string>
#include <vector>

class Command
{
public:
	typedef std::function<void(const std::string& user, const std::string& channel, const std::vector<std::string>& params)> callback_type;
	Command(const std::string& commandName, const std::string& desc, callback_type callback);
	~Command(void);

	void operator()(const std::string& user, const std::string& channel, const std::vector<std::string>& params) const;
	const std::string& name(void) const;
	const std::string& desc(void) const;
private:
	std::string mCommand;
	std::string mDescription;
	callback_type mCallback;

};

#endif
