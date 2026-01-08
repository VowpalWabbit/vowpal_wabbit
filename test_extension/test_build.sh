#!/bin/bash
set -e

echo "=== Building simple test extension ==="
mkdir -p build
cd build
cmake ..
cmake --build .

echo ""
echo "=== Testing import ==="
python3 -c "import sys; sys.path.insert(0, '.'); import simple_module; print(simple_module.hello())"

echo ""
echo "SUCCESS: Simple extension works!"
