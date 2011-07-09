#ifndef __WENDY_PROJECT_HPP__
#define __WENDY_PROJECT_HPP__

#include <wendy/common.hpp>

#include <string>
#include <list>

namespace wendy {

class AssetReader;
struct AssetNotification;
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
		 * received. When something is received, changes will be processed and
		 * listeners will be called in the current thread.
		 */
		void waitChanges();
		
		/**
		 * \brief Process received asset changes, if any
		 *
		 * This method is non-blocking. If no changes were received,
		 * nothing happens and the function returns instantly.
		 *
		 * When some changes were received, listeners will be called in the
		 * current thread.
		 */
		void checkChanges();
		
		void plop();
		
	private:
		void processNotification(const AssetNotification& notification);
		
		AssetReader *reader;
		ProjectListener *listener;
		LocalStream *stream;
};

} // wendy namespace

#endif // __WENDY_PROJECT_HPP__

