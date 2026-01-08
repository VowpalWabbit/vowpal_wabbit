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
echo "=== Testing import ==="
python3 -c "import sys; sys.path.insert(0, '.'); import simple_module; print(simple_module.hello())" || { echo "Import failed"; exit 1; }

echo ""
echo "SUCCESS: Simple extension works!"
