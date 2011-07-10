#include <iostream>
#include <string>
#include <wendy/ConditionVariable.hpp>
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
			this->counter = 0;
			this->mutex = new wendy::Mutex;
			this->cond = this->mutex->createConditionVariable();
			
			// start dumb printer thread
			this->thread = new wendy::Thread(this);
			
			// wait
			wendy::Thread::sleepSeconds(2);
			
			// ask something
			{
				std::string plop;
				wendy::ScopeLock lock(this->mutex);
				std::cout << "question: ";
				std::cin >> plop;
			}
			
			// wait for the third talking message
			{
				wendy::ScopeLock lock(this->mutex);
				while (this->counter < 3)
					this->cond->wait();
				
				std::cout << "third dumb message printed!" << std::endl;
			}
			
			// wait for the fourth talking message
			{
				wendy::ScopeLock lock(this->mutex);
				while (this->counter < 4)
					this->cond->wait();
				
				std::cout << "fourth dumb message printed!" << std::endl;
			}
			
			wendy::Thread::sleepSeconds(2);
			
			this->running = false;
		}
		
		~DualDumb()
		{
			// join thread
			delete this->thread;
			
			this->mutex->destroyConditionVariable(this->cond);
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
					
					counter++;
					this->cond->signal();
				}
				wendy::Thread::sleepSeconds(1);
			}
		}
		
		bool running;
		int counter;
		wendy::ConditionVariable *cond;
		wendy::Mutex *mutex;
		wendy::Thread *thread;
};

int main()
{
	DualDumb dumb;
	
	return 0;
}

