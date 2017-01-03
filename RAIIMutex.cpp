#include "RAIIMutex.h"

std::map<const void*, std::mutex> RAIIMutex::oMutexMap;
std::mutex RAIIMutex::oMapLock;

RAIIMutex::RAIIMutex(const void* memoryLocation):
	mMemoryLocation(memoryLocation)
{
	oMapLock.lock();
	std::mutex* lock = &oMutexMap[mMemoryLocation];
	oMapLock.unlock();

	lock->lock();
}

RAIIMutex::~RAIIMutex(void)
{
	oMapLock.lock();
	std::mutex* lock = &oMutexMap[mMemoryLocation];
	oMapLock.unlock();

	lock->unlock();
}
