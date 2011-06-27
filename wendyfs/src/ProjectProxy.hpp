#ifndef __PROJECTPROXY_HPP__
#define __PROJECTPROXY_HPP__

#include <string>
#include <vector>
#include <wendy/Project.hpp>
#include <wendy/ProjectListener.hpp>

class ProjectProxy: public wendy::ProjectListener
{
	public:
		ProjectProxy();
		~ProjectProxy();
		
		std::vector<std::string> listDirectory(std::string name);
	
	private:
		wendy::Project *project;
};

#endif //  __PROJECTPROXY_HPP__
