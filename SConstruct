import os

libEnv = Environment(tools = ["default"], ENV = os.environ)
libEnv.Append(CPPPATH = ["libwendy/include"])
libEnv.Append(CPPDEFINES = ["BUILDING_WENDY"])
if libEnv["CC"] == "cl":
	libEnv.Append(CPPFLAGS = ["/Z7", "/W2", "/wd4251", "/EHsc"])
	libEnv.Append(LIBS = ["ws2_32"])
else:
	libEnv.Append(CPPFLAGS = ["-O2", "-Wall"])

if os.name == "posix":
	libEnv.Append(LIBS = ["pthread"])
libEnv.SharedLibrary("libwendy/lib/wendy", libEnv.Glob("libwendy/src/wendy/*.cpp"))

testEnv = Environment(tools = ["default"], ENV = os.environ)
testEnv.Append(CPPPATH = ["libwendy/include"])
testEnv.Append(LIBPATH = ["libwendy/lib"])
testEnv.Append(LIBS = ["wendy"])
if testEnv["CC"] == "cl":
	testEnv.Append(CPPFLAGS = ["/EHsc"])
testEnv.Program("libwendy/tests/bin/list", "libwendy/tests/src/list.cpp")
testEnv.Program("libwendy/tests/bin/threading", "libwendy/tests/src/threading.cpp")

fsEnv = Environment(tools = ["default"], ENV = os.environ)
fsEnv.Append(CPPPATH = ["libwendy/include"])
fsEnv.Append(LIBPATH = ["libwendy/lib"])
fsEnv.Append(LIBS = ["wendy"])
if fsEnv["CC"] == "cl":
	fsEnv.Append(CPPFLAGS = ["/Z7", "/W2", "/wd4251", "/EHsc"])
else:
	fsEnv.Append(CPPFLAGS = ["-O2", "-Wall"])

if os.name == "nt":
	fsEnv.Append(CPPPATH = ["C:\\Program Files (x86)\\Dokan\\DokanLibrary"])
	fsEnv.Append(LIBPATH = ["C:\\Program Files (x86)\\Dokan\\DokanLibrary"])
	fsEnv.Append(CPPDEFINES = ["UNICODE", "_UNICODE"])
	fsEnv.Append(LIBS = ["dokan"])

fsEnv.Program("wendyfs/bin/wendyfs", fsEnv.Glob("wendyfs/src/*.cpp"))
