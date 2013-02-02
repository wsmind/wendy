import os

# User input
#buildVariables = Variables("localconfig.py")
#if os.name == "nt":
#	buildVariables.Add(PathVariable("BINARYDEPS", "Folder containing Wendy binary dependencies (windows-only)", "../wendy-deps"))

# Parallel build
SetOption('num_jobs', 4)

# Include cache
SetOption('implicit_cache', 1)

# Output directory
buildName = "posix"
if os.name == "nt":
	buildName = "win32"
buildDir = "build-" + buildName + "/"

# Common base environment
toolchain = ["default"]
if os.name == "nt":
	toolchain = ["mingw"]

baseEnvironment = Environment(tools = toolchain, BUILDDIR = buildDir)
if baseEnvironment["CC"] == "cl":
	baseEnvironment.Append(CPPFLAGS = ["/Z7", "/W2", "/wd4251", "/EHsc"])
else:
	baseEnvironment.Append(CPPFLAGS = ["-g", "-Wall", "-Werror"])

Export("baseEnvironment")

# Build win32 dependencies
if os.name == "nt":
	from subprocess import call
	call("make -C dependencies/curl mingw32")
	baseEnvironment.Command("$BUILDDIR/bin/libcurl.dll", "dependencies/curl/lib/libcurl.dll", Copy("$TARGET", "$SOURCE"))

# cJSON
cJSONObject = baseEnvironment.SharedObject("$BUILDDIR/obj/cJSON/cJSON", "dependencies/cJSON/cJSON.c")
cJSONLibrary = baseEnvironment.SharedLibrary("$BUILDDIR/bin/cJSON", cJSONObject)

# wendy subsystems
baseEnvironment.SConscript("libwendy/SConscript")
baseEnvironment.SConscript("wendyfs/SConscript")
