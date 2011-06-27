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

std::vector<std::string> ProjectProxy::listDirectory(std::string name)
{
	// filter assets by directory
	std::vector<std::string> result;
	result.push_back("plop");
	result.push_back("haha");
	
	return result;
}
