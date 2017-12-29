# -*- coding: utf-8 -*-
"""Vowpal Wabbit setup module """

import platform
import subprocess
import sys
from codecs import open
from ctypes.util import find_library
from distutils.command.clean import clean as _clean
from os import environ, makedirs, path, remove, walk
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
from setuptools.command.test import test as _test
from setuptools.command.install_lib import install_lib as _install_lib
from shutil import copy, copytree, rmtree, ignore_patterns


system = platform.system()
here = path.abspath(path.dirname(__file__))
pylibvw = Extension('pylibvw', sources=['python/pylibvw.cc'])

def removesilent(path):
    try:
        remove(path)
    except OSError:
        pass

def prep():
    """Prepare source directories for building extension """

    subprocess.check_call(['git', 'checkout-index', '--all', '--prefix', 'python/src/'], cwd=path.join(here, '..'))
    # some pruning
    for d in ['cs', 'java', 'test', 'demo']:
        rmtree(path.join(here, 'src', d), ignore_errors=False)

class Clean(_clean):
    """Clean up after building python package directories """
    def run(self):
	exit()
        removesilent(path.join(here, '.coverage'))

        rmtree(path.join(here, '.cache'), ignore_errors=True)
        rmtree(path.join(here, '.tox'), ignore_errors=True)
        rmtree(path.join(here, 'src'), ignore_errors=True)
        rmtree(path.join(here, 'dist'), ignore_errors=True)
        rmtree(path.join(here, 'build'), ignore_errors=True)
        rmtree(path.join(here, 'vowpalwabbit.egg-info'), ignore_errors=True)
        _clean.run(self)

class Sdist(_sdist):
    def run(self):
        # try to run prep if needed
        try:
            prep()
        except:
            pass
        _sdist.run(self)

class VWBuildExt(_build_ext):
    """Build pylibvw.so and install it as a python extension """
    def build_extension(self, ext):
	# prep()
        target_dir = path.join(here, path.dirname(self.get_ext_fullpath(ext.name)))
        if not path.isdir(target_dir):
            makedirs(target_dir)
        if system == 'Windows':
            if sys.version_info[0] == 2 and sys.version_info[1] == 7:
               copy(path.join(here, 'bin', 'pyvw27.dll'), self.get_ext_fullpath(ext.name))
            elif sys.version_info[0] == 3 and sys.version_info[1] == 5:
               copy(path.join(here, 'bin', 'pyvw35.dll'), self.get_ext_fullpath(ext.name))
            elif sys.version_info[0] == 3 and sys.version_info[1] == 6:
               copy(path.join(here, 'bin', 'pyvw36.dll'), self.get_ext_fullpath(ext.name))
            else:
               raise Exception('Pre-built vw/python library for Windows is not supported for this python version')
        else:
            cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + target_dir,
                          '-DPYTHON_EXECUTABLE=' + sys.executable,
                          '-DPIP_INSTALL=true']

            subprocess.check_call(['cmake', '.'] + cmake_args, cwd=path.join(here, 'src'))
            subprocess.check_call(['cmake', '--build', '.', '--', '-j'], cwd=path.join(here, 'src'))

class InstallLib(_install_lib):
    def build(self):
       _install_lib.build(self)
       if system == 'Windows':
           copy(path.join(here, 'bin', 'zlib.dll'), path.join(self.build_dir, 'zlib.dll'))

class Tox(_test):
    """ Run tox tests with 'python setup.py test' """
    tox_args = None
    test_args = None
    test_suite = None

    user_options = [('tox-args=', 'a', "Arguments to pass to tox")]

    def initialize_options(self):
        _test.initialize_options(self)
        self.tox_args = None

    def finalize_options(self):
        _test.finalize_options(self)
        self.test_args = []
        self.test_suite = True

    def run_tests(self):
        # import here, cause outside the eggs aren't loaded
        import tox
        import shlex
        args = self.tox_args
        if args:
            args = shlex.split(self.tox_args)
        errno = tox.cmdline(args=args)
        sys.exit(errno)


# Get the long description from the README file
with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='vowpalwabbit',
    # TODO: use setup py
    version='8.5.0',
    description='Vowpal Wabbit Python package',
    long_description=long_description,
    url='https://github.com/JohnLangford/vowpal_wabbit',
    author='Scott Graham',
    author_email='scott.d.graham@gmail.com',
    license='BSD 3-Clause License',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Information Analysis',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
    ],
    keywords='fast machine learning online classification regression',
    packages=find_packages(exclude=['examples', 'src', 'tests']),
    platforms='any',
    zip_safe=False,
    include_package_data=True,
    ext_modules=[pylibvw],
    cmdclass={
        'build_ext': VWBuildExt,
        'clean': Clean,
        'sdist': Sdist,
        'test': Tox,
        'install_lib': InstallLib
    },
    # tox.ini handles additional test dependencies
    tests_require=['tox']
)
