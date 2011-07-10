#ifndef __PROJECTPROXY_HPP__
#define __PROJECTPROXY_HPP__

#include <string>
#include <vector>
#include <map>
#include <wendy/Project.hpp>
#include <wendy/ProjectListener.hpp>

class ProjectProxy: public wendy::ProjectListener
{
	public:
		ProjectProxy();
		~ProjectProxy();
		
		virtual void assetAdded(wendy::Project *project, const wendy::Asset &asset);
		virtual void assetUpdated(wendy::Project *project, const wendy::Asset &asset);
		virtual void assetRemoved(wendy::Project *project, const wendy::Asset &asset);
		
		std::vector<std::string> listDirectory(std::string name);
	
	private:
		wendy::Project *project;
		
		// cross-reference assets by path
		std::map<std::string, wendy::Asset> assets;
};

#endif //  __PROJECTPROXY_HPP__
