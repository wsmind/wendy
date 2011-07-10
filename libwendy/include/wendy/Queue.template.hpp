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

