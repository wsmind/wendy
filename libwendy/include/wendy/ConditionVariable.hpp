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

