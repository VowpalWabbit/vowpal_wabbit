from setuptools import setup, Extension
import sysconfig

# Simple extension (pure Python C API)
simple_module = Extension(
    'simple_module',
    sources=['simple_module.c'],
    include_dirs=[sysconfig.get_path('include')]
)

# Boost.Python extension
boost_module = Extension(
    'boost_module',
    sources=['boost_module.cc'],
    include_dirs=[sysconfig.get_path('include')],
    libraries=['boost_python3'],
    extra_link_args=['-Wl,-undefined,dynamic_lookup'] if sysconfig.get_platform().startswith('macosx') else []
)

setup(
    name='test_extensions',
    version='0.0.1',
    description='Test extensions to isolate macOS symbol resolution issue',
    ext_modules=[simple_module, boost_module],
)
