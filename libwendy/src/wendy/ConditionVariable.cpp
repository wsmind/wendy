#include <wendy/ConditionVariable.hpp>

namespace wendy {

#ifdef WIN32
ConditionVariable::ConditionVariable(CRITICAL_SECTION *criticalSection)
{
	this->criticalSection = criticalSection;
	this->event = CreateEvent(NULL, FALSE, FALSE, NULL);
}
#else
ConditionVariable::ConditionVariable(pthread_mutex_t *posixMutex)
{
	this->posixMutex = posixMutex;
	pthread_cond_init(&this->posixCondition, NULL);
}
#endif

ConditionVariable::~ConditionVariable()
{
#	ifdef WIN32
		CloseHandle(this->event);
#	else
		pthread_cond_destroy(&this->positionCondition);
#	endif
}

void ConditionVariable::wait()
{
#	ifdef _WIN32
		LeaveCriticalSection(this->criticalSection);
		WaitForSingleObject(this->event, INFINITE);
		EnterCriticalSection(this->criticalSection);
#	else
		pthread_cond_wait(&this->posixCondition, this->posixMutex);
#	endif
}

void ConditionVariable::signal()
{
#	ifdef _WIN32
		SetEvent(this->event);
#	else
		pthread_cond_signal(&this->posixCondition);
#	endif
}

} // wendy namespace

