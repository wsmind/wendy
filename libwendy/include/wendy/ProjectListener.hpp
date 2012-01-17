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

#ifndef __WENDY_PROJECTLISTENER_HPP__
#define __WENDY_PROJECTLISTENER_HPP__

#include <wendy/common.hpp>

namespace wendy {

struct Asset;
class Project;

/**
 * \interface ProjectListener
 */
class WENDYAPI ProjectListener
{
	public:
		virtual ~ProjectListener() {}
		
		//virtual void connected() {}
		//virtual void disconnected() {}
		
		virtual void assetChanged(const Asset &asset) {}
		virtual void assetOpened(const std::string &id, unsigned long fd) {}
		virtual void chunkReceived(unsigned long fd, unsigned long offset, const void *buffer, unsigned long length) {}
};

} // wendy namespace

#endif // __WENDY_PROJECTLISTENER_HPP__
