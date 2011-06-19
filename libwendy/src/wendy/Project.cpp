#include <wendy/Project.hpp>
#include <wendy/ProjectListener.hpp>

namespace wendy {

Project::Project(const std::string &name)
{
	this->name = name;
}

void Project::connect()
{
}

void Project::disconnect()
{
}

void Project::addListener(ProjectListener *listener)
{
	this->listeners.push_back(listener);
}

void Project::removeListener(ProjectListener *listener)
{
	this->listeners.remove(listener);
}

} // wendy namespace

