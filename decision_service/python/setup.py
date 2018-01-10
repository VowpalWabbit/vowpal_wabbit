# -*- coding: utf-8 -*-
"""Vowpal Wabbit DS setup module """

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
from shutil import copy, copyfile, copytree, rmtree, ignore_patterns

system = platform.system()
here = path.abspath(path.dirname(__file__))
pylibvw = Extension('pylibvw', sources=['python/pylibvw.cc'])

# def prep():
#    """Prepare source directories for building extension """

    # rmtree(path.join(here, 'src'), ignore_errors=True)

    # subprocess.check_call(['git', 'checkout-index', '--all', '--prefix', 'python/src/'], cwd=path.join(here, '..'))

    # some pruning
    # for d in ['cs', 'java', 'test', 'demo', 'logo_assets']:
    #    rmtree(path.join(here, 'src', d), ignore_errors=False)

class Sdist(_sdist):
    def run(self):
        # try to run prep if needed
        # prep()
        _sdist.run(self)

class VWBuildExt(_build_ext):
    """Build pylibvw.so and install it as a python extension """
    def build_extension(self, ext):
	    # moving the binary into install folder to make allow for multiple versions
        # , 'vowpalwabbitds'
        outdir = 'python{version}/swig{version}'.format(version=sys.version_info[0])
        libname = '_decision_service{version}.so'.format(version=sys.version_info[0])

        target_dir = path.join(here, path.dirname(self.get_ext_fullpath(ext.name)))
        if not path.isdir(target_dir):
            makedirs(target_dir)
        
        lib = path.join(here, outdir, libname)
        if path.isfile(lib):
            # if library has already been built (e.g. through conda)
            copyfile(lib, path.join(target_dir, libname))
        else:
            cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + target_dir]

            subprocess.check_call(['cmake', '.'] + cmake_args, cwd=path.join(here, 'src'))
            subprocess.check_call(['cmake', '--build', '.', '--', '-j'], cwd=path.join(here, 'src'))

setup(
    name='vowpalwabbitds',
    version='8.5.1', # TODO: setup.py.in
    description='Vowpal Wabbit DS Python package',
    url='https://github.com/JohnLangford/vowpal_wabbit',
    author='Markus Cozowicz',
    author_email='marcozo@microsoft.com',
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
    # packages=find_packages(exclude=['examples', 'src', 'tests']),
    package_dir = {'': 'python{version}/swig{version}'.format(version = sys.version_info[0])},
    py_modules = ['decision_service'],
    platforms='any',
    zip_safe=False,
    include_package_data=True,
    ext_modules=[pylibvw],
    cmdclass={
        'build_ext': VWBuildExt
        #'clean': Clean,
        #'sdist': Sdist,
    }
)
