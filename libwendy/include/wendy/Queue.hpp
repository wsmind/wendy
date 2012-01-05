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

#ifndef __WENDY_QUEUE_HPP__
#define __WENDY_QUEUE_HPP__

#include <wendy/common.hpp>
#include <list>

namespace wendy {

class ConditionVariable;
class Mutex;

/**
 * \class Queue
 * \brief Synchronized FIFO queue
 */
template <class PayloadType>
class Queue
{
	public:
		/**
		 * \brief Constructor
		 */
		Queue();
		
		/**
		 * \brief Destructor
		 */
		~Queue();
		
		/**
		 * \brief Test if receive can be called without blocking
		 *
		 * Will return true either if elements are present in the queue, or
		 * if the termination event was sent
		 */
		bool isEmpty();
		
		/**
		 * \brief Send an element through the queue
		 */
		void send(const PayloadType &element);
		
		/**
		 * \brief Send termination event
		 *
		 * This will make the next call to receive() return true.
		 */
		void sendTermination();
		
		/**
		 * \brief Pops the first element available and return it
		 * \return true if sendTermination() was called, and false otherwise
		 *
		 * Blocks until at least one element is available. If isEmpty() returned false,
		 * the next call to this method is guaranteed not to block.
		 *
		 * When true is returned (termination event), no useful data is put in
		 * the payload object.
		 */
		bool receive(PayloadType *element);
		
	private:
		std::list<PayloadType> internalQueue;
		wendy::Mutex *mutex;
		wendy::ConditionVariable *conditionVariable;
		
		// used for ending payload queueing
		bool terminated;
};

} // wendy namespace

#include <wendy/Queue.template.hpp>

#endif //  __WENDY_QUEUE_HPP__

