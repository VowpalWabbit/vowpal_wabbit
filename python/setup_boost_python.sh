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

# Append Python configuration to project-config.jam (created by bootstrap.sh)
cat >> project-config.jam << EOF

# Python configuration for current version
import python ;
using python
    : $PYTHON_VERSION
    : $PYTHON_EXEC
    : $PYTHON_INCLUDE
    : $PYTHON_LIBDIR
    ;
EOF

echo "project-config.jam contents:"
cat project-config.jam

# Build and install only boost.python
echo "Building boost.python..."
./b2 --with-python install -j$(sysctl -n hw.ncpu) 2>&1 | tail -100
