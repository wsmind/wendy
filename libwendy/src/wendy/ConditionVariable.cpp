/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011 Remi Papillie
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 * 
 *****************************************************************************/

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
		pthread_cond_destroy(&this->posixCondition);
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

