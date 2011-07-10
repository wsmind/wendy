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

#ifndef __WENDY_MUTEX_HPP__
#define __WENDY_MUTEX_HPP__

#include <wendy/common.hpp>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <pthread.h>
#endif

namespace wendy {

class ConditionVariable;

class WENDYAPI Mutex
{
	public:
		Mutex();
		~Mutex();
		
		void acquire();
		void release();
		
		/**
		 * \brief Create a condition variable associated with this mutex
		 */
		ConditionVariable *createConditionVariable();
		
		void destroyConditionVariable(ConditionVariable *variable);
		
	private:
#		ifdef _WIN32
			CRITICAL_SECTION criticalSection;
#		else
			pthread_mutex_t posixMutex;
#		endif
};

} // wendy namespace

#endif //  __WENDY_MUTEX_HPP__

