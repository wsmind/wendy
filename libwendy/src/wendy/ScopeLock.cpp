#include <wendy/ScopeLock.hpp>

#include <wendy/Mutex.hpp>

#include <iostream>

namespace wendy {

ScopeLock::ScopeLock(Mutex *mutex)
{
	this->mutex = mutex;
	this->mutex->acquire();
}

ScopeLock::~ScopeLock()
{
	this->mutex->release();
}

} // wendy namespace

