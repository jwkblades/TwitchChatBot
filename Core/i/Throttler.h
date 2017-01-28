#ifndef __THROTTLE_H
#define __THROTTLE_H

#include <chrono>
#include <list>

class Throttler
{
public:
	Throttler(std::chrono::high_resolution_clock::duration timeLimit = std::chrono::seconds(30));
	~Throttler(void);

	bool check(std::size_t num) const;

	void addUnit(void);
private:
	mutable std::list<std::chrono::time_point<std::chrono::high_resolution_clock>> mOccurrences;
	std::chrono::high_resolution_clock::duration mTimeLimit;
};

#endif
