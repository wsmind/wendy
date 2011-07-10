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

#ifndef __WENDY_ASSET_HPP__
#define __WENDY_ASSET_HPP__

#include <wendy/common.hpp>
#include <string>

namespace wendy {

/**
 * \class Asset
 * \brief Unique data structure of wendy
 *
 * This structure holds information both about the remote and local states
 * of the asset (e.g the last version available, and the current download state)
 */
struct WENDYAPI Asset
{
	/// unique id (will remain the same even in case of path changes)
	std::string id;
	
	/// relative path to repository root (contains forward slashes only)
	std::string path;
	
	/// user locking this asset ("" if not locked)
	//std::string lockingUser;
};

} // wendy namespace

#endif // __WENDY_ASSET_HPP__

