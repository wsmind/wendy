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

