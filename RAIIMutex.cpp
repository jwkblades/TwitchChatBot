#include "RAIIMutex.h"

RAIIMutex::RAIIMutex(const void* memoryLocation):
	mMemoryLocation(memoryLocation)
{
	retrieveLockFor(mMemoryLocation)->lock();
}

RAIIMutex::~RAIIMutex(void)
{
	retrieveLockFor(mMemoryLocation)->unlock();
}


std::mutex* RAIIMutex::retrieveLockFor(const void* location)
{
	static std::map<const void*, std::mutex*> oMutexMap;
	static std::mutex oMapLock;

	oMapLock.lock();
	std::map<const void*, std::mutex*>::iterator found = oMutexMap.find(location);
	if (found == oMutexMap.end())
	{
		oMutexMap.insert({location, new std::mutex()});
	}
	std::mutex* lock = oMutexMap[location];
	oMapLock.unlock();

	return lock;
}
