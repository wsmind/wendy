import os

# User input
#buildVariables = Variables("localconfig.py")
#if os.name == "nt":
#	buildVariables.Add(PathVariable("BINARYDEPS", "Folder containing Wendy binary dependencies (windows-only)", "../wendy-deps"))

# Parallel build
SetOption('num_jobs', 4)

# Include cache
SetOption('implicit_cache', 1)

useMingw = True

# Output directory
buildName = "posix"
if os.name == "nt":
	if useMingw:
		buildName = "win32-mingw"
	else:
		buildName = "win32-vc"
buildDir = "build-" + buildName + "/"

# Common base environment
toolchain = ["default"]
if os.name == "nt" and useMingw:
	toolchain = ["mingw"]

baseEnvironment = Environment(tools = toolchain, BUILDDIR = buildDir, ENV = os.environ)
if baseEnvironment["CC"] == "cl":
	baseEnvironment.Append(CPPFLAGS = ["/Z7", "/W2", "/wd4251", "/EHsc"])
else:
	baseEnvironment.Append(CPPFLAGS = ["-g", "-Wall", "-Werror"])

Export("baseEnvironment")

# Build win32 dependencies
if os.name == "nt":
	from subprocess import call
	if useMingw:
		call("make -C dependencies/curl mingw32")
	else:
		call("cd dependencies/curl && nmake vc-dll", shell = True)
	baseEnvironment.Command("$BUILDDIR/bin/libcurl.dll", "dependencies/curl/lib/libcurl.dll", Copy("$TARGET", "$SOURCE"))

# cJSON
cJSONObject = baseEnvironment.SharedObject("$BUILDDIR/obj/cJSON/cJSON", "dependencies/cJSON/cJSON.c")
cJSONLibrary = baseEnvironment.SharedLibrary("$BUILDDIR/bin/cJSON", cJSONObject)

# wendy subsystems
baseEnvironment.SConscript("libwendy/SConscript")
baseEnvironment.SConscript("wendyfs/SConscript")
