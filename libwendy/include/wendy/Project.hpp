#ifndef __WENDY_PROJECT_HPP__
#define __WENDY_PROJECT_HPP__

#include <wendy/common.hpp>

#include <string>
#include <list>

namespace wendy {

class LocalStream;
class ProjectListener;

/**
 * \class Project
 */
class WENDYAPI Project
{
	public:
		Project(ProjectListener *listener);
		~Project();
		
		void connect();
		void disconnect();
		bool isConnected();
		
	private:
		ProjectListener *listener;
		LocalStream *stream;
};

} // wendy namespace

#endif // __WENDY_PROJECT_HPP__

