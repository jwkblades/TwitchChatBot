#include "Throttler.h"

Throttler::Throttler(std::chrono::high_resolution_clock::duration timeLimit):
	mOccurrences(),
	mTimeLimit(timeLimit)
{
	// empty
}

Throttler::~Throttler(void)
{
	// empty
}

bool Throttler::check(std::size_t num) const
{
	auto currentTime = std::chrono::high_resolution_clock::now() - mTimeLimit;

	std::list<std::chrono::time_point<std::chrono::high_resolution_clock>>::iterator iter = mOccurrences.begin();
	for (; iter != mOccurrences.end(); iter++)
	{
		if (*iter >= currentTime)
		{
			break;
		}
	}
	mOccurrences.erase(mOccurrences.begin(), iter);

	return mOccurrences.size() < num;
}

void Throttler::addUnit(void)
{
	mOccurrences.push_back(std::chrono::high_resolution_clock::now());
}
