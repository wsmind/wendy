@echo off
set PATH=%PATH%;../lib

for %%f in (bin\*.exe) do echo === %%f === && %%f
