#include <wendy/Project.hpp>
#include <wendy/ProjectListener.hpp>

class TestPlop: public wendy::ProjectListener
{
	public:
		virtual void assetAdded(wendy::Project *project, wendy::Asset *asset)
		{
		}
		
		virtual void assetRemoved(wendy::Project *project, wendy::Asset *asset)
		{
		}
};

int main()
{
	wendy::Project *project = new wendy::Project("plop");
	TestPlop *listener = new TestPlop;
	project->addListener(listener);
	
	project->connect();
	project->disconnect();
	
	project->removeListener(listener);
	delete listener;
	delete project;
}

