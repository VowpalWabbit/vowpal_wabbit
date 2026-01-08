#!/bin/bash
set -e

echo "=== Python version ==="
python3 --version

echo ""
echo "=== Building simple test extension ==="
mkdir -p build
cd build
cmake .. || { echo "CMake configuration failed"; exit 1; }
cmake --build . || { echo "Build failed"; exit 1; }

echo ""
echo "=== Extension file ==="
ls -la simple_module.*.so || ls -la simple_module.so || { echo "Extension file not found"; exit 1; }

echo ""
echo "=== Testing simple_module import ==="
python3 -c "import sys; sys.path.insert(0, '.'); import simple_module; print(simple_module.hello())" || { echo "simple_module import failed"; exit 1; }

echo ""
echo "=== Checking for boost_module ==="
if ls boost_module.*.so >/dev/null 2>&1 || ls boost_module.so >/dev/null 2>&1; then
    echo "boost_module found - testing import"
    ls -la boost_module.*.so || ls -la boost_module.so
    echo ""
    echo "=== Testing boost_module import ==="
    python3 -c "import sys; sys.path.insert(0, '.'); import boost_module; print(boost_module.greet())" || { echo "boost_module import failed"; exit 1; }
    echo ""
    echo "SUCCESS: Both simple and Boost.Python extensions work!"
else
    echo "boost_module not built (Boost not found) - skipping"
    echo ""
    echo "SUCCESS: Simple extension works!"
fi
