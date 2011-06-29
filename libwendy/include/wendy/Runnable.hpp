#ifndef __WENDY_RUNNABLE_HPP__
#define __WENDY_RUNNABLE_HPP__

#include <wendy/common.hpp>

namespace wendy {

/**
 * \interface Runnable
 * \brief Asynchronous processing
 */
class WENDYAPI Runnable
{
	public:
		/**
		 * \brief Destructor
		 */
		virtual ~Runnable() {}
		
		/**
		 * \brief Processing method
		 */
		void run() {}
};

} // wendy namespace

#endif //  __WENDY_RUNNABLE_HPP__

