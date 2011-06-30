#ifndef __WENDY_THREAD_HPP__
#define __WENDY_THREAD_HPP__

#include <wendy/common.hpp>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <unistd.h>
#	include <pthread.h>
#endif

namespace wendy {

class Runnable;

/**
 * \class Thread
 * \brief Basic thread class
 */
class WENDYAPI Thread
{
	public:
		/**
		 * \brief Constructor
		 *
		 * Execute the runnable run() method asynchronously.
		 */
		Thread(Runnable *runnable);
		
		/**
		 * \brief Destructor
		 *
		 * Join and destroy the thread
		 */
		~Thread();
		
		/**
		 * \brief Put the current thread to sleep temporarily
		 */
		static void sleepSeconds(unsigned int seconds);
		
	private:
#		ifdef _WIN32
			/// windows-specific wrapper function
			static DWORD WINAPI winThreadRunner(LPVOID lpParameter);
			
			/// windows-specific thread handle
			HANDLE winHandle;
#		else
			static void *posixThreadRunner(void *data);
			pthread_t posixThread;
#		endif
};

} // wendy namespace

#endif //  __WENDY_THREAD_HPP__

