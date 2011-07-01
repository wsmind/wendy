#include <wendy/Mutex.hpp>

namespace wendy {

Mutex::Mutex()
{
#	ifdef _WIN32
		InitializeCriticalSection(&this->criticalSection);
#	else
		pthread_mutex_init(&this->posixMutex, NULL);
#	endif
}

Mutex::~Mutex()
{
#	ifdef _WIN32
		DeleteCriticalSection(&this->criticalSection);
#	else
		pthread_mutex_destroy(&this->posixMutex);
#	endif
}

void Mutex::acquire()
{
#	ifdef _WIN32
		EnterCriticalSection(&this->criticalSection);
#	else
		pthread_mutex_lock(&this->posixMutex);
#	endif
}

void Mutex::release()
{
#	ifdef _WIN32
		LeaveCriticalSection(&this->criticalSection);
#	else
		pthread_mutex_unlock(&this->posixMutex);
#	endif
}

} // wendy namespace

