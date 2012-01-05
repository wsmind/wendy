/******************************************************************************
 * 
 * Wendy asset manager
 * Copyright (c) 2011 Remi Papillie
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 * 
 *****************************************************************************/

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
			// send special termination message
			this->queue->sendTermination();
			
			// join thread
			std::cout << "joining" << std::endl;
			delete this->thread;
			std::cout << "joined" << std::endl;
			
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
				wendy::Thread::sleepSeconds(1);
				std::string thing;
				bool terminated = this->queue->receive(&thing);
				
				if (terminated)
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
	wendy::Thread::sleepSeconds(5);
	proc.process("goofy");
	
	return 0;
}

