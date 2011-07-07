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

