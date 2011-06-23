#include <wendy/Project.hpp>

#include <iostream>
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

void Project::plop()
{
	std::string line;
	while (this->stream->readLine(&line))
	{
		std::cout << "line: " << line << std::endl;
	}
}

} // wendy namespace

