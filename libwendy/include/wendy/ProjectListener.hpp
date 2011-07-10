#ifndef __WENDY_PROJECTLISTENER_HPP__
#define __WENDY_PROJECTLISTENER_HPP__

#include <wendy/common.hpp>

namespace wendy {

struct Asset;
class Project;

/**
 * \interface ProjectListener
 */
class WENDYAPI ProjectListener
{
	public:
		virtual void assetAdded(Project *project, const Asset &asset) {}
		virtual void assetUpdated(Project *project, const Asset &asset) {}
		virtual void assetRemoved(Project *project, const Asset &asset) {}
};

} // wendy namespace

#endif // __WENDY_PROJECTLISTENER_HPP__

