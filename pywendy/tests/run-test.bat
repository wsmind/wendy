@echo off

rem This script sets up the environment for running sample test
rem python scripts that make use of locally compiled versions
rem of libwendy and pywendy libraries.

rem find local libwendy
set PATH=%PATH%;..\..\libwendy\lib

rem find local pywendy
set PYTHONPATH=..\lib

python %1 %2 %3 %4 %5 %6 %7 %8 %9
