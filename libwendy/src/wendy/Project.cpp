#include <wendy/Project.hpp>

#include <wendy/LocalStream.hpp>
#include <wendy/ProjectListener.hpp>

namespace wendy {

Project::Project(ProjectListener *listener)
{
	this->listener = listener;
	this->stream = NULL;
}

Project::~Project()
{
	delete this->stream;
}

void Project::connect()
{
	this->stream = new LocalStream(46288);
}

void Project::disconnect()
{
	delete this->stream;
	this->stream = NULL;
}

bool Project::isConnected()
{
	return this->stream && this->stream->isConnected();
}

} // wendy namespace

