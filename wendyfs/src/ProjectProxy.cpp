#include "ProjectProxy.hpp"

#include <wendy/Project.hpp>

ProjectProxy::ProjectProxy(const std::string name)
{
	this->project = new wendy::Project(name);
	this->project->addListener(this);
}

std::vector<std::string> ProjectProxy::listDirectory(std::string name)
{
	// filter assets by directory
	std::vector<std::string> result;
	result.push_back("plop");
	result.push_back("haha");
	
	return result;
}
