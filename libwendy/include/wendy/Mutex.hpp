#ifndef __WENDY_MUTEX_HPP__
#define __WENDY_MUTEX_HPP__

#include <wendy/common.hpp>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <pthread.h>
#endif

namespace wendy {

class WENDYAPI Mutex
{
	public:
		Mutex();
		~Mutex();
		
		void acquire();
		void release();
		
	private:
#		ifdef _WIN32
			CRITICAL_SECTION criticalSection;
#		else
			pthread_mutex_t posixMutex;
#		endif
};

} // wendy namespace

#endif //  __WENDY_MUTEX_HPP__

