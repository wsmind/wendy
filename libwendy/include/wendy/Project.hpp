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

#ifndef __WENDY_PROJECT_HPP__
#define __WENDY_PROJECT_HPP__

#include <wendy/common.hpp>

#include <string>
#include <list>
#include <map>

#include <wendy/Asset.hpp>
#include <wendy/AssetFile.hpp>

namespace wendy {

struct AssetNotification;
class AssetReader;
class LocalStream;
class ProjectListener;

/**
 * \class Project
 */
class WENDYAPI Project
{
	public:
		Project(ProjectListener *listener);
		~Project();
		
		void connect();
		void disconnect();
		bool isConnected();
		
		/**
		 * \brief Block current thread until new information about asset arrives
		 *
		 * Will block until at least one new information packet about an asset is
		 * received. When something is received, changes will be processed and
		 * listeners will be called in the current thread.
		 */
		void waitChanges();
		
		/**
		 * \brief Process received asset changes, if any
		 *
		 * This method is non-blocking. If no changes were received,
		 * nothing happens and the function returns instantly.
		 *
		 * When some changes were received, listeners will be called in the
		 * current thread.
		 */
		void checkChanges();
		
		const Asset getAsset(const std::string &id);
		
		void createAsset(const std::string &path);
		
		void deleteAsset(const std::string &id);
		
		void lockAsset(const std::string &id, const std::string &applicationName);
		
		void unlockAsset(const std::string &id);
		
		AssetFile *openAsset(const std::string &id, AssetFile::OpenMode mode);
		
		void closeAsset(AssetFile *file);
		
	private:
		void processNotification(const AssetNotification& notification);
		
		AssetReader *reader;
		ProjectListener *listener;
		LocalStream *stream;
		
		typedef std::map<std::string, Asset> AssetMap;
		AssetMap assets;
};

} // wendy namespace

#endif // __WENDY_PROJECT_HPP__

