#include <wendy/Mutex.hpp>

#include <wendy/ConditionVariable.hpp>

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

ConditionVariable *Mutex::createConditionVariable()
{
#	ifdef _WIN32
		return new ConditionVariable(&this->criticalSection);
#	else
		return new ConditionVariable(&this->posixMutex);
#	endif
}

void Mutex::destroyConditionVariable(ConditionVariable *variable)
{
	delete variable;
}

} // wendy namespace

