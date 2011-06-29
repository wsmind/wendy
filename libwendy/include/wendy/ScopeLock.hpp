#ifndef __WENDY_SCOPELOCK_HPP__
#define __WENDY_SCOPELOCK_HPP__

#include <wendy/common.hpp>

namespace wendy {

class Mutex;

/**
 * \class ScopeLock
 * \brief Scope-based (RAII) mutex locking
 */
class WENDYAPI ScopeLock
{
	public:
		/**
		 * \brief Constructor
		 *
		 * The mutex is acquired during construction
		 */
		ScopeLock(Mutex *mutex);
		
		/**
		 * \brief Destructor
		 *
		 * The mutex is released during destruction
		 */
		~ScopeLock();
		
	private:
		Mutex *mutex;
};

} // wendy namespace

#endif //  __WENDY_SCOPELOCK_HPP__

