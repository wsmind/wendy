#ifndef __WENDY_THREAD_HPP__
#define __WENDY_THREAD_HPP__

#include <wendy/common.hpp>

#ifdef _WIN32
#	include <windows.h>
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
		
	private:
#		ifdef _WIN32
			/// windows-specific wrapper function
			static DWORD WINAPI threadRunner(LPVOID lpParameter);
			
			/// windows-specific thread handle
			HANDLE handle;
#		endif
};

} // wendy namespace

#endif //  __WENDY_THREAD_HPP__

