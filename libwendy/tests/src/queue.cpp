#include <iostream>
#include <string>
#include <wendy/Mutex.hpp>
#include <wendy/Queue.hpp>
#include <wendy/Runnable.hpp>
#include <wendy/ScopeLock.hpp>
#include <wendy/Thread.hpp>

class Processor: public wendy::Runnable
{
	public:
		Processor()
		{
			this->queue = new wendy::Queue<std::string>;
			
			// start processing thread
			this->thread = new wendy::Thread(this);
		}
		
		~Processor()
		{
			// send termination message
			this->queue->send("exit");
			
			// join thread
			delete this->thread;
			
			delete this->queue;
		}
		
		void process(std::string thing)
		{
			this->queue->send(thing);
		}
		
		virtual void run()
		{
			bool running = true;
			while (running)
			{
				std::string thing = this->queue->receive();
				
				if (thing == "exit")
				{
					std::cout << "stopped processing!!" << std::endl;
					running = false;
				}
				else
				{
					std::cout << "processing '" << thing << "'" << std::endl;
				}
			}
		}
		
		wendy::Queue<std::string> *queue;
		wendy::Thread *thread;
};

int main()
{
	Processor proc;
	
	proc.process("aladdin");
	proc.process("mickey");
	proc.process("goofy");
	
	return 0;
}

