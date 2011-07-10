#include "ProjectProxy.hpp"

#include <wendy/Project.hpp>

ProjectProxy::ProjectProxy()
{
	this->project = new wendy::Project(this);
	this->project->connect();
}

ProjectProxy::~ProjectProxy()
{
	this->project->disconnect();
}

void ProjectProxy::assetAdded(wendy::Project *project, const wendy::Asset &asset)
{
	this->assets[asset.path] = asset;
}

void ProjectProxy::assetUpdated(wendy::Project *project, const wendy::Asset &asset)
{
	// TODO: delete old path
	
	this->assets[asset.path] = asset;
}

void ProjectProxy::assetRemoved(wendy::Project *project, const wendy::Asset &asset)
{
	this->assets.erase(asset.path);
}

std::vector<std::string> ProjectProxy::listDirectory(std::string name)
{
	// update local asset list
	this->project->checkChanges();
	
	// filter assets by directory
	std::vector<std::string> result;
	
	// currently return all assets systematically
	std::map<std::string, wendy::Asset>::iterator it;
	for (it = this->assets.begin(); it != this->assets.end(); ++it)
		result.push_back(it->first);
	
	return result;
}
