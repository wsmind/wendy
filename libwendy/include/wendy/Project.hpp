#ifndef __WENDY_PROJECT_HPP__
#define __WENDY_PROJECT_HPP__

#include <wendy/common.hpp>

#include <string>
#include <list>

namespace wendy {

class ProjectListener;

/**
 * \class Project
 */
class WENDYAPI Project
{
	public:
		Project(const std::string &name);
		
		void connect();
		void disconnect();
		
		void addListener(ProjectListener *listener);
		void removeListener(ProjectListener *listener);
		
	private:
		std::string name;
		std::list<ProjectListener *> listeners;
};

} // wendy namespace

#endif // __WENDY_PROJECT_HPP__

