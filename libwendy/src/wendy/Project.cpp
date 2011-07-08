#include <wendy/Project.hpp>

#include <iostream>
#include <wendy/AssetReader.hpp>
#include <wendy/LocalStream.hpp>
#include <wendy/ProjectListener.hpp>

namespace wendy {

Project::Project(ProjectListener *listener)
{
	this->listener = listener;
	this->stream = NULL;
	this->reader = NULL;
}

Project::~Project()
{
	delete this->stream;
}

void Project::connect()
{
	this->stream = new LocalStream(46288);
	this->reader = new AssetReader(this->stream);
}

void Project::disconnect()
{
	delete this->reader;
	this->reader = NULL;
	
	delete this->stream;
	this->stream = NULL;
}

bool Project::isConnected()
{
	return this->stream && this->stream->isConnected();
}

void Project::waitChanges()
{
}

void Project::checkChanges()
{
}

void Project::plop()
{
	std::string line;
	while (this->stream->readLine(&line))
	{
		std::cout << "line: " << line << std::endl;
		std::cin >> line;
		this->stream->writeLine(line);
	}
}

} // wendy namespace

