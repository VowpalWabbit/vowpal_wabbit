# -*- coding: utf-8 -*-
""" Vowpal Wabbit python setup module """

import distutils.dir_util
import os
import platform
import sys
from codecs import open
from distutils.command.clean import clean as _clean
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
from setuptools.command.test import test as _test
from setuptools.command.install_lib import install_lib as _install_lib
from shutil import copy, rmtree

system = platform.system()
version_info = sys.version_info
here = os.path.abspath(os.path.dirname(__file__))

class CMakeExtension(Extension):
    def __init__(self, name):
        # don't invoke the original build_ext for this special extension
        Extension.__init__(self, name, sources=[])

class BuildPyLibVWBindingsModule(_build_ext):

    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        _build_ext.run(self)

    def build_cmake(self, ext):
        # Make build directory
        distutils.dir_util.mkpath(self.build_temp)

        # Ensure lib output directory is made
        lib_output_dir = os.path.join(here, os.path.dirname(self.get_ext_fullpath(ext.name)))
        distutils.dir_util.mkpath(lib_output_dir)

        # example of cmake args
        config = 'Debug' if self.debug else 'Release'
        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + str(lib_output_dir),
            '-DCMAKE_BUILD_TYPE=' + config,
            '-DPY_VERSION=' + '{v[0]}.{v[1]}'.format(v=version_info),
            '-DWARNINGS=Off'
        ]

        # example of build args
        build_args = [
            '--config', config,
            '--', '-j8',
            # Build the pylibvw target
            "pylibvw"
        ]

        cmake_directory = os.path.join(here, '..')
        os.chdir(str(self.build_temp))
        self.spawn(['cmake', str(cmake_directory)] + cmake_args)
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.'] + build_args)
        os.chdir(str(here))


class Clean(_clean):
    """ Clean up after building python package directories """
    def run(self):
        rmtree(os.path.join(here, 'dist'), ignore_errors=True)
        rmtree(os.path.join(here, 'build'), ignore_errors=True)
        rmtree(os.path.join(here, 'vowpalwabbit.egg-info'), ignore_errors=True)
        _clean.run(self)


class Sdist(_sdist):
    def run(self):
        # run prep if needed
        prep()
        _sdist.run(self)


class InstallLib(_install_lib):
    def build(self):
        _install_lib.build(self)
        if system == 'Windows':
            copy(os.path.join(here, 'bin', 'zlib.dll'), os.path.join(self.build_dir, 'zlib.dll'))


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
with open(os.path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

# Get the current version for the python package from the configure.ac file
config_path = os.path.join(here, '..', 'version.txt')
with open(config_path, encoding='utf-8') as f:
    version = f.readline().strip()

setup(
    name='vowpalwabbit',
    version=version,
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
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    ],
    keywords='fast machine learning online classification regression',
    packages=find_packages(),
    platforms='any',
    zip_safe=False,
    include_package_data=True,
    ext_modules=[CMakeExtension('pylibvw')],
    cmdclass={
        'build_ext': BuildPyLibVWBindingsModule,
        'clean': Clean,
        'sdist': Sdist,
        'test': Tox,
        'install_lib': InstallLib
    },
    # tox.ini handles additional test dependencies
    tests_require=['tox']
)
