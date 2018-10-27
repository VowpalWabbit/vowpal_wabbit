```
/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
```

<img src="/logo_assets/vowpal-wabbits-github-logo@3x.png" height="auto" width="100%" alt="Vowpal Wabbit">

[![Build Status](https://travis-ci.org/JohnLangford/vowpal_wabbit.png)](https://travis-ci.org/JohnLangford/vowpal_wabbit)
[![Windows Build Status](https://ci.appveyor.com/api/projects/status/github/JohnLangford/vowpal_wabbit?branch=master&svg=true)](https://ci.appveyor.com/project/JohnLangford/vowpal-wabbit)
[![Coverage Status](https://coveralls.io/repos/JohnLangford/vowpal_wabbit/badge.svg)](https://coveralls.io/r/JohnLangford/vowpal_wabbit)
[![Total Alerts](https://img.shields.io/lgtm/alerts/g/JohnLangford/vowpal_wabbit.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/JohnLangford/vowpal_wabbit/alerts/)

This is the *vowpal wabbit* fast online learning code.  For Windows, look at [README.windows.txt](./README.windows.txt)

## Prerequisite software

These prerequisites are usually pre-installed on many platforms. However, you may need to consult your favorite package
manager (*yum*, *apt*, *MacPorts*, *brew*, ...) to install missing software.

- [Boost](http://www.boost.org) library, (for `Boost::Program_Options` and a few others).
- The zlib compression library + headers. In linux distros: package `zlib-devel` (Red Hat/CentOS), or `zlib1g-dev` (Ubuntu/Debian)
- lsb-release  (RedHat/CentOS: redhat-lsb-core, Debian: lsb-release, Ubuntu: you're all set, OSX: not required)
- GNU *autotools*: *autoconf*, *automake*, *libtool*, *autoheader*, et. al. This is not a strict prereq. On many systems (notably Ubuntu with `libboost-program-options-dev` installed), the provided `Makefile` works fine.
- (optional) [git](http://git-scm.com) if you want to check out the latest version of *vowpal wabbit*,
  work on the code, or even contribute code to the main project.

## Getting the code

You can download the latest version from [here](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Download).
The very latest version is always available via 'github' by invoking one of the following:

```
## For the traditional ssh-based Git interaction:
$ git clone git://github.com/VowpalWabbit/vowpal_wabbit.git

## You can also try the following SSH URL:
$ git clone git@github.com:VowpalWabbit/vowpal_wabbit.git

## For HTTP-based Git interaction
$ git clone https://github.com/VowpalWabbit/vowpal_wabbit.git
```

## Compiling

You should be able to build the *vowpal wabbit* on most systems with:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make test    # (optional)
```

If that fails, try:
```
$ ./autogen.sh
$ make
$ make test    # (optional)
$ make install
```

Note that `./autogen.sh` requires *automake* (see the prerequisites, above.)

`./autogen.sh`'s command line arguments are passed directly to `configure` as
if they were `configure` arguments and flags.

Note that `./autogen.sh` will overwrite the supplied `Makefile`, including the `Makefile`s in sub-directories, so
keeping a copy of the `Makefile`s may be a good idea before running `autogen.sh`. If your original `Makefile`s were overwritten by `autogen.sh` calling `automake`, you may always get the originals back from git using:
```
git checkout Makefile */Makefile
```

Be sure to read the wiki: https://github.com/VowpalWabbit/vowpal_wabbit/wiki
for the tutorial, command line options, etc.

The 'cluster' directory has it's own documentation for cluster
parallel use, and the examples at the end of test/Runtests give some
example flags.

## C++ Optimization

The default C++ compiler optimization flags are very aggressive. If you should run into a problem, consider creating and running `configure` with the `--enable-debug` option, e.g.:

```
$ ./configure --enable-debug
```

or passing your own compiler flags via the `OPTIM_FLAGS` make variable:

```
$ make OPTIM_FLAGS="-O0 -g"
```

## Ubuntu/Debian specific info

On Ubuntu/Debian/Mint and similar the following sequence should work
for building the latest from github:

```
# -- Get libboost and zlib:
apt-get install libboost-dev zlib1g-dev

# -- Get the python libboost bindings (python subdir) - optional:
apt-get install libboost-python-dev

# -- Get the vw source:
git clone git://github.com/VowpalWabbit/vowpal_wabbit.git

# -- Build:
cd vowpal_wabbit
make
make test       # (optional)
make install
```

### Ubuntu advanced build options (clang and static)

If you prefer building with `clang` instead of `gcc` (much faster build
and slighly faster executable), install `clang` and change the `make`
step slightly:

```
apt-get install clang

make CXX=clang++
```

A statically linked `vw` executable that is not sensitive to boost
version upgrades and can be safely copied between different Linux
versions (e.g. even from Ubuntu to Red-Hat) can be built and tested with:

```
make CXX='clang++ -static' clean vw test     # ignore warnings
```

## Debian Python 3 Binding

Ensure boost-library and c-compiler are installed:
```
apt-get install libboost-dev zlib1g-dev libboost-python-dev clang make automake
```

Set Python 3.x and its boost-library as default:
```
update-alternatives --install /usr/bin/python python /usr/bin/python2.7 1
update-alternatives --install /usr/bin/python python /usr/bin/python3.x 2

ln -sf /usr/lib/x86_64-linux-gnu/libboost_python-py3x.a /usr/lib/x86_64-linux-gnu/libboost_python.a
ln -sf /usr/lib/x86_64-linux-gnu/libboost_python-py3x.so /usr/lib/x86_64-linux-gnu/libboost_python.so
```

Install Vowpal Wabbit via pip:
```
pip3 install vowpalwabbit
```


## Mac OS X-specific info

OSX requires _glibtools_, which is available via the [brew](http://brew.sh) or
[MacPorts](https://www.macports.org) package managers.

### Complete brew install of 8.4
```
brew install vowpal-wabbit
```
[The homebrew formula for VW is located on github](https://github.com/Homebrew/homebrew-core/blob/master/Formula/vowpal-wabbit.rb).

### Manual install of Vowpal Wabbit
#### OSX Dependencies (if using Brew):
```
brew install libtool
brew install autoconf
brew install automake
brew install boost
brew install boost-python
```

#### OSX Dependencies (if using MacPorts):
```
## Install glibtool and other GNU autotool friends:
$ port install libtool autoconf automake

## Build Boost for Mac OS X 10.8 and below
$ port install boost +no_single -no_static +openmpi +python27 configure.cxx_stdlib=libc++ configure.cxx=clang++

## Build Boost for Mac OS X 10.9 and above
$ port install boost +no_single -no_static +openmpi +python27
```

#### OSX Manual compile:
*Mac OS X 10.8 and below*: ``configure.cxx_stdlib=libc++`` and ``configure.cxx=clang++`` ensure that ``clang++`` uses
the correct C++11 functionality while building Boost. Ordinarily, ``clang++`` relies on the older GNU ``g++`` 4.2 series
header files and ``stdc++`` library; ``libc++`` is the ``clang`` replacement that provides newer C++11 functionality. If
these flags aren't present, you will likely encounter compilation errors when compiling _vowpalwabbit/cbify.cc_. These
error messages generally contain complaints about ``std::to_string`` and ``std::unique_ptr`` types missing.

To compile:
```
$ sh autogen.sh --enable-libc++
$ make
$ make test    # (optional)
```

#### OSX Python Binding installation with Anaconda
When using Anaconda as the source for Python the default Boost libraries used in the Makefile need to be adjusted. Below are the steps needed to install the Python bindings for VW. This should work for Python 2 and 3. Adjust the directories to match where anaconda is installed.

```
# create anaconda environment with boost
conda create --name vw boost
source activate vw
git clone https://github.com/VowpalWabbit/vowpal_wabbit.git
cd vowpal_wabbit
# edit Makefile
# change BOOST_INCLUDE to use anaconda env dir: /anaconda/envs/vw/include
# change BOOST_LIBRARY to use anaconda lib dir: /andaconda/envs/vw/lib
cd python
python setup.py install
```

## Code Documentation
To browse the code more easily, do

`make doc`

and then point your browser to `doc/html/index.html`.

Note that documentation generates class diagrams using [Graphviz](https://www.graphviz.org). For best results, ensure that it is installed beforehand.


## Experimental: CMake build system
Note: The CSharp projects are not yet converted to CMake for Windows. So the CMake generated solution is only for C++ projects for the time being. For this reason the existing solution can not yet be deprecated.
### Dependencies
```
vcpkg install rapidjson:x64-linux
vcpkg install cpprestsdk:x64-linux
vcpkg install zlib:x64-linux
vcpkg install boost-system:x64-linux
vcpkg install boost-program-options:x64-linux
vcpkg install boost-test:x64-linux
vcpkg install boost-align:x64-windows
vcpkg install boost-foreach:x64-windows
```

### Build
#### Windows
1. Open CMake GUI
2. Add two entries
    1. `CMAKE_TOOLCHAIN_FILE=<vcpkg root>/scripts/buildsystems/vcpkg.cmake`
    2. `VCPKG_TARGET_TRIPLET=x64-windows`
    3. `CMAKE_BUILD_TYPE=DEBUG`
3. Configure
    1. Choose `Visual Studio 15 2017 Win64`
4. Generate
5. Open Project

#### Linux
Note: not yet tested
```
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux -DCMAKE_BUILD_TYPE=DEBUG
make -j
```
