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
	static std::map<const void*, std::mutex> oMutexMap;
	static std::mutex oMapLock;
	const void* mMemoryLocation;
};

#endif
