#include <wendy/Thread.hpp>

#include <wendy/Runnable.hpp>

namespace wendy {

Thread::Thread(Runnable *runnable)
{
#	ifdef _WIN32
		this->handle = CreateThread(0, 0, Thread::threadRunner, (void *)runnable, 0, NULL);
#	endif
}

Thread::~Thread()
{
#	ifdef _WIN32
	WaitForSingleObject(this->handle, INFINITE);
	CloseHandle(this->handle);
#	endif
}

#ifdef _WIN32
DWORD WINAPI Thread::threadRunner(LPVOID lpParameter)
{
	Runnable *runnable = (Runnable *)lpParameter;
	runnable->run();
	
	return 0;
}
#endif

} // wendy namespace

