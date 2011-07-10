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

#include <wendy/Thread.hpp>

#include <wendy/Runnable.hpp>

namespace wendy {

Thread::Thread(Runnable *runnable)
{
#	ifdef _WIN32
		this->winHandle = CreateThread(0, 0, Thread::winThreadRunner, (void *)runnable, 0, NULL);
#	else
		pthread_create(&this->posixThread, NULL, Thread::posixThreadRunner, (void *)runnable);
#	endif
}

Thread::~Thread()
{
#	ifdef _WIN32
		WaitForSingleObject(this->winHandle, INFINITE);
		CloseHandle(this->winHandle);
#	else
		pthread_join(this->posixThread, NULL);
#	endif
}

void Thread::sleepSeconds(unsigned int seconds)
{
#	ifdef _WIN32
		Sleep(seconds * 1000);
#	else
		sleep(seconds);
#	endif
}

#ifdef _WIN32
DWORD WINAPI Thread::winThreadRunner(LPVOID lpParameter)
{
	Runnable *runnable = (Runnable *)lpParameter;
	runnable->run();
	
	return 0;
}
#else
void *Thread::posixThreadRunner(void *data)
{
	Runnable *runnable = (Runnable *)data;
	runnable->run();
	
	return NULL;
}
#endif

} // wendy namespace

