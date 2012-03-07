import os

# Parallel build
SetOption('num_jobs', 4)

# Include cache
SetOption('implicit_cache', 1)

buildName = "posix"
if os.name == "nt":
	buildName = "win32"

buildDir = "build-" + buildName + "/"

# Common base environment
toolchain = ["default"]
if os.name == "nt":
	toolchain = ["mingw"]
baseEnvironment = Environment(tools = toolchain, BUILDDIR=buildDir)
Export("baseEnvironment")

# Each SConscript return the deliverables to put in the dist/ folder
# in the form of an item list: [ (path, SCons node), (path, SCons node), ... ]
distItems = []
distItems += baseEnvironment.SConscript("daemon/SConscript", variant_dir="$BUILDDIR/daemon", duplicate=0)
distItems += baseEnvironment.SConscript("libwendy/SConscript", variant_dir="$BUILDDIR/libwendy", duplicate=0)
distItems += baseEnvironment.SConscript("pywendy/SConscript", variant_dir="$BUILDDIR/pywendy", duplicate=0)
distItems += baseEnvironment.SConscript("wendyfs/SConscript", variant_dir="$BUILDDIR/wendyfs", duplicate=0)

print(distItems)

# Package in dist/ folder
distEnv = Environment(BUILDDIR=buildDir)
for path, node in distItems:
	print("$BUILDDIR/dist/" + path)
	distEnv.Command("$BUILDDIR/dist/" + path, node, Copy("$TARGET", "$SOURCE"))

#distEnv.Command("$BUILDDIR/dist/" + distEnv["SHLIBPREFIX"] + "wendy" + distEnv["SHLIBSUFFIX"], distEnv.Glob("$BUILDDIR/libwendy/lib/*.so"), Copy("$TARGET", "$SOURCE"))
#distEnv.Command("$BUILDDIR/dist/" + "wendyfs" + distEnv["PROGSUFFIX"], distEnv.Glob("$BUILDDIR/wendyfs/bin/*"), Copy("$TARGET", "$SOURCE"))
#for file in distEnv.Glob("daemon/*.js"):
#	distEnv.Command(os.path.join("$BUILDDIR/dist/daemon", file.name), file, Copy("$TARGET", "$SOURCE"))
