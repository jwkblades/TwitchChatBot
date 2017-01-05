#include "ScopeExit.h"

ScopeExit::ScopeExit(ScopeExitRun whenToRun):
	mWhenToRun(whenToRun),
	mCallback(nullptr)
{
	// empty
}

ScopeExit::~ScopeExit(void)
{
	int exceptions = std::uncaught_exception();
	if (mWhenToRun == RUN_ALWAYS ||
			(mWhenToRun == RUN_ON_FAILURE && exceptions > 0) ||
			(mWhenToRun == RUN_ON_SUCCESS && exceptions == 0)
	   )
	{
		mCallback();
	}
}

void ScopeExit::operator=(std::function<void(void)> callback)
{
	mCallback = callback;
}
