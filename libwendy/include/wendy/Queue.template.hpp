#ifndef __WENDY_QUEUE_TEMPLATE_HPP__
#define __WENDY_QUEUE_TEMPLATE_HPP__

#include <wendy/ConditionVariable.hpp>
#include <wendy/Mutex.hpp>
#include <wendy/ScopeLock.hpp>

namespace wendy {

template <class PayloadType>
Queue<PayloadType>::Queue()
{
	this->mutex = new wendy::Mutex;
	this->conditionVariable = this->mutex->createConditionVariable();
}

template <class PayloadType>
Queue<PayloadType>::~Queue()
{
	this->mutex->destroyConditionVariable(this->conditionVariable);
	delete this->mutex;
}

template <class PayloadType>
bool Queue<PayloadType>::isEmpty()
{
	ScopeLock lock(this->mutex);
	
	return (this->internalQueue.size() == 0);
}

template <class PayloadType>
void Queue<PayloadType>::send(const PayloadType &element)
{
	ScopeLock lock(this->mutex);
	
	this->internalQueue.push_back(element);
	this->conditionVariable->signal();
}

template <class PayloadType>
PayloadType Queue<PayloadType>::receive()
{
	ScopeLock lock(this->mutex);
	
	// wait for at least an element
	while (this->internalQueue.size() == 0)
		this->conditionVariable->wait();
	
	// send it back
	PayloadType element = this->internalQueue.front();
	this->internalQueue.pop_front();
	return element;
}

} // wendy namespace

#endif //  __WENDY_QUEUE_TEMPLATE_HPP__

