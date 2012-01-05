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

#include <iostream>
#include <wendy/Project.hpp>
#include <wendy/ProjectListener.hpp>

class TestPlop: public wendy::ProjectListener
{
	public:
		virtual void assetAdded(wendy::Project *project, const wendy::Asset &asset)
		{
			std::cout << "Added: " << asset.id << " at " << asset.path << std::endl;
		}
		
		virtual void assetUpdated(wendy::Project *project, const wendy::Asset &oldAsset, const wendy::Asset &newAsset)
		{
			std::cout << "Updated: " << oldAsset.id;
		}
		
		virtual void assetRemoved(wendy::Project *project, const wendy::Asset &asset)
		{
			std::cout << "Removed: " << asset.id << " at " << asset.path << std::endl;
		}
};

int main()
{
	TestPlop *listener = new TestPlop;
	wendy::Project *project = new wendy::Project(listener);
	
	project->connect();
	
	if (!project->isConnected())
		std::cout << "Connection failed!" << std::endl;
	
	while (project->isConnected())
	{
		project->waitChanges();
	}
	
	project->disconnect();
	
	delete listener;
	delete project;
}
