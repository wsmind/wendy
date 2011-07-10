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

