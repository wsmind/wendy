#!/bin/sh

# This script sets up the environment for running sample test
# python scripts that make use of locally compiled versions
# of libwendy and pywendy libraries.

if [ $# -lt "1" ]
then
	echo "Usage: $0 <py file>"
	exit
fi

# find local libwendy
export LD_LIBRARY_PATH=../../libwendy/lib

# find local pywendy
export PYTHONPATH=../lib

python $*

