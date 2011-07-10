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

