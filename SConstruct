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
testEnv.Program("libwendy/tests/bin/add", "libwendy/tests/src/add.cpp")
testEnv.Program("libwendy/tests/bin/list", "libwendy/tests/src/list.cpp")
testEnv.Program("libwendy/tests/bin/queue", "libwendy/tests/src/queue.cpp")
testEnv.Program("libwendy/tests/bin/threading", "libwendy/tests/src/threading.cpp")

pylibEnv = Environment(tools = ["default"], ENV = os.environ)
pylibEnv.Append(CPPPATH = ["libwendy/include"])
pylibEnv.Append(LIBPATH = ["libwendy/lib"])
pylibEnv.Append(LIBS = ["wendy"])
pylibEnv.Append(SWIGFLAGS = ["-c++"])
pylibEnv.Append(SWIGFLAGS = ["-python"])
pylibEnv.Append(SWIGFLAGS = ["-Ilibwendy/include"])
pylibEnv.Append(SWIGFLAGS = ["-outdir", "pywendy/lib"])
pylibEnv["SHLIBPREFIX"] = "" # remove the leading 'lib' from output library filename
if os.name == "posix":
	# TODO: remove hard-coded python path
	pylibEnv.Append(CPPPATH = ["/usr/include/python2.6"])
pylibEnv.SharedLibrary("pywendy/lib/_pywendy", libEnv.Glob("pywendy/src/*.i"))

fsEnv = Environment(tools = ["default"], ENV = os.environ)
fsEnv.Append(CPPPATH = ["libwendy/include"])
fsEnv.Append(LIBPATH = ["libwendy/lib"])
fsEnv.Append(LIBS = ["wendy"])
if fsEnv["CC"] == "cl":
	fsEnv.Append(CPPFLAGS = ["/Z7", "/W2", "/wd4251", "/EHsc"])
else:
	fsEnv.Append(CPPFLAGS = ["-O2", "-Wall"])

if os.name == "nt":
	# TODO: remove hard-coded dokan path
	fsEnv.Append(CPPPATH = ["C:\\Program Files (x86)\\Dokan\\DokanLibrary"])
	fsEnv.Append(LIBPATH = ["C:\\Program Files (x86)\\Dokan\\DokanLibrary"])
	fsEnv.Append(CPPDEFINES = ["UNICODE", "_UNICODE"])
	fsEnv.Append(LIBS = ["dokan"])
else:
	fsEnv.ParseConfig("pkg-config --cflags --libs fuse")
	fsEnv.Append(CPPDEFINES = "FUSE_USE_VERSION=28")

fsEnv.Program("wendyfs/bin/wendyfs", fsEnv.Glob("wendyfs/src/*.cpp"))
