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
		 * \brief Test if elements are present in the queue (non-blocking)
		 */
		bool isEmpty();
		
		/**
		 * \brief Send an element through the queue
		 */
		void send(const PayloadType &element);
		
		/**
		 * \brief Pops the first element available and return it
		 *
		 * Blocks until at least one element is available. If isEmpty() returned false,
		 * the next call to this method is guaranteed not to blocK.
		 */
		PayloadType receive();
		
	private:
		std::list<PayloadType> internalQueue;
		wendy::Mutex *mutex;
		wendy::ConditionVariable *conditionVariable;
};

} // wendy namespace

#include <wendy/Queue.template.hpp>

#endif //  __WENDY_QUEUE_HPP__

