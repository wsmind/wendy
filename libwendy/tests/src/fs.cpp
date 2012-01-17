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
#include <wendy/ProjectFileSystem.hpp>

#include <wendy/Thread.hpp>

int main()
{
	wendy::ProjectFileSystem fs;
	
	wendy::Thread::sleepSeconds(2);
	
	std::vector<std::string> files;
	fs.readdir("", &files);
	
	for (unsigned int i = 0; i < files.size(); i++)
		std::cout << "found file: " << files[i] << std::endl;
	
	long fd = fs.open("portal2.wav", wendy::ProjectFileSystem::READING);
	std::cout << "fd of 'portal2.wav': " << fd << std::endl;
	
	char buf[100];
	if (fs.read(fd, 10, buf, 100))
		std::cout << "read succeeded!" << std::endl;
	else
		std::cout << "read failed!" << std::endl;
	
	fs.close(fd);
	
	return 0;
}
