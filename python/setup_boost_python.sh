#!/bin/bash
# Script to configure and build boost.python for the current Python version

set -e

BOOST_DIR="/tmp/boost_build/boost_1_85_0"
INSTALL_PREFIX="/tmp/boost_install"

cd "$BOOST_DIR"

# Get Python configuration
PYTHON_VERSION=$(python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
PYTHON_EXEC=$(which python)
PYTHON_INCLUDE=$(python -c "import sysconfig; print(sysconfig.get_path('include'))")
PYTHON_LIBDIR=$(python -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))")

echo "Configuring boost.python for Python $PYTHON_VERSION"
echo "  Executable: $PYTHON_EXEC"
echo "  Include: $PYTHON_INCLUDE"
echo "  Libdir: $PYTHON_LIBDIR"

# Create project-config.jam
cat > project-config.jam << EOF
import python ;

if ! [ python.configured ]
{
    using python
        : $PYTHON_VERSION
        : $PYTHON_EXEC
        : $PYTHON_INCLUDE
        : $PYTHON_LIBDIR
        ;
}
EOF

echo "project-config.jam created:"
cat project-config.jam

# Build and install
echo "Building boost.python..."
./b2 --debug-configuration install -j$(sysctl -n hw.ncpu)
