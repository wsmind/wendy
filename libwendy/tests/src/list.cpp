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
	TestPlop *listener = new TestPlop;
	wendy::Project *project = new wendy::Project(listener);
	
	project->connect();
	
	while (project->isConnected())
	{
		//project->wait();
	}
	
	project->disconnect();
	
	delete listener;
	delete project;
}

