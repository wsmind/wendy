/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011-2012 Remi Papillie
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

#include <wendy/RequestState.hpp>

#include <cassert>

namespace wendy {

RequestState::RequestState()
{
	this->state = NOT_STARTED;
}

RequestState::~RequestState()
{
	assert(this->state != IN_PROGRESS);
}

bool RequestState::isFinished() const
{
	return (this->state == SUCCEEDED) || (this->state == FAILED);
}

bool RequestState::isSuccess() const
{
	return this->state == SUCCEEDED;
}

void RequestState::start()
{
	assert(this->state == NOT_STARTED);
	this->state = IN_PROGRESS;
}

void RequestState::succeed()
{
	this->state = SUCCEEDED;
}

void RequestState::fail()
{
	this->state = FAILED;
}

} // wendy namespace
