#!/bin/bash
set -e

echo "=== Python version ==="
python3 --version

echo ""
echo "=== Building wheel ==="
python3 -m pip install build delocate -q --break-system-packages
python3 -m build --wheel

echo ""
echo "=== Wheel file ==="
ls -la dist/*.whl

echo ""
echo "=== Running delocate-wheel ==="
delocate-wheel -w repaired_wheels -v dist/*.whl

echo ""
echo "=== Repaired wheel ==="
ls -la repaired_wheels/*.whl

echo ""
echo "=== Installing repaired wheel ==="
python3 -m pip install repaired_wheels/*.whl --force-reinstall -q --break-system-packages

echo ""
echo "=== Testing simple_module import after delocate-wheel ==="
python3 -c "import simple_module; print(simple_module.hello())" || { echo "simple_module import FAILED after delocate-wheel"; exit 1; }

echo ""
echo "=== Testing boost_module import after delocate-wheel ==="
python3 -c "import boost_module; print(boost_module.greet())" || { echo "boost_module import FAILED after delocate-wheel"; exit 1; }

echo ""
echo "SUCCESS: Extensions work after delocate-wheel!"
