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

#ifndef __WENDY_HTTPWRITER_HPP__
#define __WENDY_HTTPWRITER_HPP__

#include <wendy/common.hpp>

#include <string>

namespace wendy {

/**
 * \interface HttpWriter
 */
class WENDYAPI HttpWriter
{
	public:
		virtual ~HttpWriter() {}
		
		virtual void writeHttpData(const char *buffer, unsigned int size) {}
};

} // wendy namespace

#endif // __WENDY_HTTPWRITER_HPP__