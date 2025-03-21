# Need to figure out which libraries need to be installed.
# Relies on GTest
# sudo apt install google-mock libgmock-dev libgtest-dev
#

BUILDDIR=build
BUILDTYPE=Release

if [ $# -eq 1 ]; then
	if [ "$1" = "debug" -o "$1" = "Debug" ]; then
		BUILDDIR=${BUILDDIR}.d
		BUILDTYPE=Debug
	elif [ "$1" = "release" -o "$1" = "Release" ]; then
		:
	else
		echo "Parameter can be: [ debug | release (default) ]"
		return 1
	fi
fi

if [ -d $BUILDDIR ]; then
	# zsh "read" is different from bash. This is zsh's "read".
	read "cmd?Delete build directory ($BUILDDIR)? [yN] "
	if [ "$cmd" = "y" -o "$cmd" = "Y" ]; then
		:
	else
		return 0
	fi
fi

rm -rf $BUILDDIR
rm -f compile_commands.json

mkdir $BUILDDIR && \
cd $BUILDDIR

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

cmake -GNinja \
	-DCMAKE_BUILD_TYPE=$BUILDTYPE \
	-DCMAKE_INSTALL_PREFIX=bin \
	..

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
cd ..
ln -s ${BUILDDIR}/compile_commands.json

# Build targets from top project directory.
#cmake --build $BUILDDIR

# Install targets by pulling build targets into $BUILDDIR/bin and bin/lib.
# This way you can quickly run it with: build/bin/camera
#cmake --install $BUILDDIR

# Run CTest tests from top project directory.
#cmake --build $BUILDDIR --target test

# Microsoft format is closest to the format that I like.
# If you use nvim, it will probably download its own clang-format binary.
#clang-format --style Microsoft --dump-config > .clang-format
#~/.local/share/nvim/mason/bin/clang-format --style Microsoft --dump-config > .clang-format
