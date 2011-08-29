###############################################################################
# 
# Wendy asset manager
# Copyright (c) 2011 Remi Papillie
# 
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
# 
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
# 
#    1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 
#    2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 
#    3. This notice may not be removed or altered from any source
#    distribution.
# 
###############################################################################

import sys
import os

import engine
import service
import storage.couch

if __name__ == "__main__":
	# check required arguments
	if len(sys.argv) < 3:
		print("Usage: " + sys.argv[0] + " <server> <project>")
		sys.exit(0)
	
	storage = storage.couch.Couch(sys.argv[1], sys.argv[2])
	e = engine.Engine(sys.argv[1], sys.argv[2])
	s = service.Server(e)
	
	# terminate the whole process if SIGTERM is received
	try:
		s.waitConnections()
	except KeyboardInterrupt:
		os._exit(1)

