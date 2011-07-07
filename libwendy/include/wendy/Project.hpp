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
		
		/**
		 * \brief Block current thread until new information about asset arrives
		 *
		 * Will block until at least one new information packet about an asset is
		 * received.
		 */
		void waitChanges();
		
		/**
		 * \brief Process received asset changes, if any
		 *
		 * This method is non-blocking. If no changes were received,
		 * nothing happens and the function returns instantly.
		 */
		void processChanges();
		
		void plop();
		
	private:
		ProjectListener *listener;
		LocalStream *stream;
};

} // wendy namespace

#endif // __WENDY_PROJECT_HPP__

