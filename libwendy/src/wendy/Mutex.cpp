#include <wendy/Mutex.hpp>

namespace wendy {

Mutex::Mutex()
{
#	ifdef _WIN32
		InitializeCriticalSection(&this->criticalSection);
#	endif
}

Mutex::~Mutex()
{
#	ifdef _WIN32
		DeleteCriticalSection(&this->criticalSection);
#	endif
}

void Mutex::acquire()
{
#	ifdef _WIN32
		EnterCriticalSection(&this->criticalSection);
#	endif
}

void Mutex::release()
{
#	ifdef _WIN32
		LeaveCriticalSection(&this->criticalSection);
#	endif
}

} // wendy namespace

