#ifndef __WENDY_QUEUE_HPP__
#define __WENDY_QUEUE_HPP__

#include <wendy/common.hpp>
#include <list>

namespace wendy {

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
		 * \brief Send an element through the queue
		 */
		void send(const PayloadType &element);
		
		/**
		 * \brief Pops the first element available and return it
		 *
		 * Blocks until at least one element is available.
		 */
		PayloadType receive();
		
	private:
		std::list<PayloadType> internalQueue;
		wendy::Mutex *mutex;
};

} // wendy namespace

#include <wendy/Queue.template.hpp>

#endif //  __WENDY_QUEUE_HPP__

