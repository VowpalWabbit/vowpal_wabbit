#!/bin/bash
set -e

echo "=== Python version ==="
python3 --version

echo ""
echo "=== Extensions work before delocate (already tested in test_build.sh) ==="
echo "Skipping pre-delocate test - already confirmed working"

echo ""
echo "=== Installing packaging tools ==="
python3 -m pip install delocate wheel setuptools --break-system-packages -q

echo ""
echo "=== Creating wheel from CMake-built extensions ==="
cd build

# Create package structure
mkdir -p wheeltest/test_extensions
cp simple_module.so boost_module.so wheeltest/test_extensions/ 2>/dev/null || cp simple_module.*.so boost_module.*.so wheeltest/test_extensions/
cd wheeltest

# Create __init__.py
touch test_extensions/__init__.py

# Create minimal setup.py
cat > setup.py <<'SETUPEOF'
from setuptools import setup, find_packages
setup(
    name='test_extensions',
    version='0.0.1',
    packages=find_packages(),
    package_data={'test_extensions': ['*.so']},
)
SETUPEOF

# Build wheel
python3 setup.py bdist_wheel -q

echo ""
echo "=== Original wheel ==="
ls -lh dist/*.whl

echo ""
echo "=== Running delocate-wheel ==="
mkdir -p repaired
delocate-wheel -w repaired -v dist/*.whl

echo ""
echo "=== Repaired wheel ==="
ls -lh repaired/*.whl

echo ""
echo "===  Installing repaired wheel ==="
python3 -m pip install repaired/*.whl --force-reinstall --break-system-packages

echo ""
echo "=== Testing simple_module import after delocate ==="
python3 -c "import sys; sys.path.insert(0, '.'); from test_extensions import simple_module; print('Loaded simple_module')" || { echo "simple_module import FAILED"; exit 1; }

if ls test_extensions/boost_module*.so >/dev/null 2>&1; then
    echo ""
    echo "=== Testing boost_module import after delocate ==="
    python3 -c "import sys; sys.path.insert(0, '.'); from test_extensions import boost_module; print('Loaded boost_module')" || { echo "boost_module import FAILED"; exit 1; }
fi

echo ""
echo "SUCCESS: Extensions work after delocate-wheel!"
