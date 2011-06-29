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
			this->mutex = new wendy::Mutex;
			
			// start dumb printer thread
			this->thread = new wendy::Thread(this);
			
			// wait
			Sleep(1000);
			
			// ask something
			{
				std::string plop;
				wendy::ScopeLock(this->mutex);
				std::cin >> plop;
			}
			
			Sleep(1000);
		}
		
		~DualDumb()
		{
			// join thread
			delete this->thread;
			
			delete this->mutex;
		}
		
		void run()
		{
			// take lock for user
			wendy::ScopeLock(this->mutex);
			
			std::cout << "i'm talking to u :p" << std::endl;
		}
		
		wendy::Mutex *mutex;
		wendy::Thread *thread;
};

int main()
{
	DualDumb dumb;
	
	return 0;
}

