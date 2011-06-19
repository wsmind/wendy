#!/usr/bin/env python

import engine
import sys

if __name__ == "__main__":
	# check required arguments
	if len(sys.argv) < 3:
		print("Usage: " + sys.argv[0] + " <server> <project>")
		sys.exit(0)
	
	e = engine.Engine(sys.argv[1], sys.argv[2])
