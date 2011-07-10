#include <wendy/ScopeLock.hpp>

#include <wendy/Mutex.hpp>

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

