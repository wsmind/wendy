#include <iostream>
#include <string>
#include <wendy/Mutex.hpp>
#include <wendy/Runnable.hpp>
#include <wendy/ScopeLock.hpp>
#include <wendy/Thread.hpp>

class DualDumb: public wendy::Runnable
{
	public:
		DualDumb()
		{
			this->running = true;
			this->mutex = new wendy::Mutex;
			
			// start dumb printer thread
			this->thread = new wendy::Thread(this);
			
			// wait
			Sleep(200);
			
			// ask something
			{
				std::string plop;
				wendy::ScopeLock lock(this->mutex);
				std::cout << "question: ";
				std::cin >> plop;
			}
			
			Sleep(200);
			
			this->running = false;
		}
		
		~DualDumb()
		{
			// join thread
			delete this->thread;
			
			delete this->mutex;
		}
		
		virtual void run()
		{
			while (this->running)
			{
				{
					// take lock for user
					wendy::ScopeLock lock(this->mutex);
					
					std::cout << "i'm talking to u :p" << std::endl;
				}
				Sleep(50);
			}
		}
		
		bool running;
		wendy::Mutex *mutex;
		wendy::Thread *thread;
};

int main()
{
	DualDumb dumb;
	
	return 0;
}

