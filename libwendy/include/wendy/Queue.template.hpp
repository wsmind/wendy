#ifndef __WENDY_QUEUE_TEMPLATE_HPP__
#define __WENDY_QUEUE_TEMPLATE_HPP__

#include <wendy/Mutex.hpp>
#include <wendy/ScopeLock.hpp>

namespace wendy {

template <class PayloadType>
Queue<PayloadType>::Queue()
{
	this->mutex = new wendy::Mutex;
}

template <class PayloadType>
Queue<PayloadType>::~Queue()
{
	delete this->mutex;
}

template <class PayloadType>
void Queue<PayloadType>::send(const PayloadType &element)
{
	ScopeLock(this->mutex);
	
	this->internalQueue.push_back(element);
}

template <class PayloadType>
PayloadType Queue<PayloadType>::receive()
{
	// AARGH
	while (1)
	{
		ScopeLock(this->mutex);
		
		if (this->internalQueue.size() > 0)
		{
			PayloadType element = this->internalQueue.front();
			this->internalQueue.pop_front();
			return element;
		}
	}
}

} // wendy namespace

#endif //  __WENDY_QUEUE_TEMPLATE_HPP__

