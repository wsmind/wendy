import os

# User input
buildVariables = Variables("localconfig.py")
if os.name == "nt":
	buildVariables.Add(PathVariable("BINARYDEPS", "Folder containing Wendy binary dependencies (windows-only)", "../wendy-deps"))

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
baseEnvironment = Environment(tools = toolchain, BUILDDIR=buildDir, variables=buildVariables)
baseEnvironment["BINARYDEPS"] = os.path.abspath(baseEnvironment["BINARYDEPS"])
Export("baseEnvironment")

# Command line help
Help(buildVariables.GenerateHelpText(baseEnvironment))

# Each SConscript return the deliverables to put in the dist/ folder
# in the form of an item list: [ (path, SCons node), (path, SCons node), ... ]
distItems = []
distItems += baseEnvironment.SConscript("daemon/SConscript", variant_dir="$BUILDDIR/daemon", duplicate=0)
distItems += baseEnvironment.SConscript("libwendy/SConscript", variant_dir="$BUILDDIR/libwendy", duplicate=0)
distItems += baseEnvironment.SConscript("pywendy/SConscript", variant_dir="$BUILDDIR/pywendy", duplicate=0)
distItems += baseEnvironment.SConscript("wendyfs/SConscript", variant_dir="$BUILDDIR/wendyfs", duplicate=0)

# Package in dist/ folder
distEnv = Environment(BUILDDIR=buildDir)
for path, node in distItems:
	distEnv.Command("$BUILDDIR/dist/" + path, node, Copy("$TARGET", "$SOURCE"))
