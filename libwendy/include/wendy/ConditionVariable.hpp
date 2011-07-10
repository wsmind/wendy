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

#ifndef __WENDY_CONDITIONVARIABLE_HPP__
#define __WENDY_CONDITIONVARIABLE_HPP__

#include <wendy/common.hpp>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <pthread.h>
#endif

namespace wendy {

class WENDYAPI ConditionVariable
{
	public:
#		ifdef WIN32
			ConditionVariable(CRITICAL_SECTION *criticalSection);
#		else
			ConditionVariable(pthread_mutex_t *posixMutex);
#		endif
		
		~ConditionVariable();
		
		void wait();
		void signal();
		
	private:
#		ifdef _WIN32
			CRITICAL_SECTION *criticalSection;
			HANDLE event;
#		else
			pthread_mutex_t *posixMutex;
			pthread_cond_t posixCondition;
#		endif
};

} // wendy namespace

#endif //  __WENDY_CONDITIONVARIABLE_HPP__

