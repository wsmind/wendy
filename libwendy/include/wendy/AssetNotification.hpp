#ifndef __WENDY_ASSETNOTIFICATION_HPP__
#define __WENDY_ASSETNOTIFICATION_HPP__

#include <wendy/common.hpp>
#include <wendy/Asset.hpp>

namespace wendy {

/**
 * \struct AssetNotification
 * \brief Embeds operations that can happen to assets
 */
struct WENDYAPI AssetNotification
{
	enum NotificationType
	{
		UPDATED,
		DELETED
	};
	
	/// what happened to this asset
	NotificationType type;
	
	/// actual asset content
	/// only the id of the asset is guaranteed to be valid
	/// other fields of the structure are usable only when type is UPDATED
	Asset asset;
};

} // wendy namespace

#endif // __WENDY_ASSETNOTIFICATION_HPP__

