#ifndef __RAII_MUTEX_H
#define __RAII_MUTEX_H

#include <map>
#include <mutex>

class RAIIMutex
{
public:
	RAIIMutex(const void* memoryLocation);
	~RAIIMutex(void);
private:
	static std::mutex* retrieveLockFor(const void* location);
	const void* mMemoryLocation;
};

#endif
