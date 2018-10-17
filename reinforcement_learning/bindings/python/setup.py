import os
import sys
import glob
import setuptools

if 'RL_PYTHON_EXT_DEPS' in os.environ:
    external_deps_dir = os.environ['RL_PYTHON_EXT_DEPS']
else:
    print("RL_PYTHON_EXT_DEPS environment variable must be set with the path to retrieve dependencies from")
    sys.exit(1)

with open('README.md', 'r') as fh:
    long_description = fh.read()

extension_module = setuptools.Extension(
  	'rl_client._rl_client',
  	sources = glob.glob('*.cc'),
  	library_dirs = [],
  	include_dirs = ['../../include/'],
  	libraries = ['pthread', 'dl'],
  	extra_compile_args = ['-std=c++11'],
  	extra_objects = ['../../rlclientlib/librlclient.a', external_deps_dir + 'libcpprest.a', '../../../vowpalwabbit/libvw.a', '../../../vowpalwabbit/liballreduce.a', \
        external_deps_dir + 'libboost_system.a', external_deps_dir + 'libboost_program_options.a', \
        external_deps_dir + 'libssl.a', external_deps_dir + 'libcrypto.a', external_deps_dir + 'libz.a']
)

setuptools.setup(
    version = '0.0.6',
    name = 'rl_client',
    url = 'https://github.com/JohnLangford/vowpal_wabbit',
    description = 'Python binding for reinforcement learning client library',
    long_description = long_description,
    author = 'Microsoft Corporation',
    author_email = 'decisionservicedevs@microsoft.com',
    license = 'MIT',
    ext_modules = [extension_module],
    py_modules = ['rl_client.py'],
    packages = setuptools.find_packages(),
    classifiers = (
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3.6',
    )
)

