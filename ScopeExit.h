#ifndef __SCOPE_EXIT_H
#define __SCOPE_EXIT_H

#include <exception>
#include <functional>

enum ScopeExitRun
{
	RUN_ON_SUCCESS = 1,
	RUN_ON_FAILURE = 2,
	RUN_ALWAYS     = 3
};

class ScopeExit
{
public:
	ScopeExit(ScopeExitRun whenToRun);
	~ScopeExit(void);
	void operator=(std::function<void(void)> callback);
private:
	ScopeExitRun mWhenToRun;
	std::function<void(void)> mCallback;
};

#define CONCAT(a, b ) a##b
#define SCOPE_HELPER(id, when) ScopeExit CONCAT(_scope_exit_, id)(when); \
	CONCAT(_scope_exit_, id) = 
#define SCOPE_EXIT SCOPE_HELPER(__COUNTER__, RUN_ALWAYS)
#define SCOPE_FAILURE SCOPE_HELPER(__COUNTER__, RUN_ON_FAILURE)
#define SCOPE_SUCCESS SCOPE_HELPER(__COUNTER__, RUN_ON_SUCCESS)

#endif
