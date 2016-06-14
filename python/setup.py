# -*- coding: utf-8 -*-
"""Vowpal Wabbit setup module """

import subprocess
import sys
from distutils.command.clean import clean as _clean
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
from setuptools.command.test import test as _test
from codecs import open
from os import makedirs, path, remove, walk
from shutil import copy, copytree, rmtree


here = path.abspath(path.dirname(__file__))
pylibvw = Extension('pylibvw', sources=['python/pylibvw.cc'])


def prep():
    """Prepare source directories for building extension """

    # helper function to exclude subdirectories during copytree calls
    def exclude_dirs(cur_dir, _):
        return next(walk(cur_dir))[1]

    # don't create src folder if it already exists
    if not path.exists(path.join(here, 'src')):
        # add main directory (exclude children to avoid recursion)
        copytree(path.join(here, '..'), path.join(here, 'src'), ignore=exclude_dirs)

        # add python directory (exclude children to avoid recursion)
        copytree(path.join(here), path.join(here, 'src', 'python'), ignore=exclude_dirs)
        subprocess.check_call(['make', 'clean'], cwd=path.join(here, 'src', 'python'))

        # add explore
        copytree(path.join(here, '..', 'explore'), path.join(here, 'src', 'explore'))

        # add folders necessary to run 'make python'
        for folder in ['library', 'vowpalwabbit']:
            copytree(path.join(here, '..', folder), path.join(here, 'src', folder))
            subprocess.check_call(['make', 'clean'], cwd=path.join(here, 'src', folder))


class Clean(_clean):
    """Clean up after building python package directories """
    def run(self):
        try:
            remove(path.join(here, '.coverage'))
        except OSError:
            pass

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
        prep()
        subprocess.check_call(['make', 'python'], cwd=path.join(here, 'src'))
        target_dir = path.dirname(self.get_ext_fullpath(ext.name))
        if not path.isdir(target_dir):
            makedirs(target_dir)
        copy(path.join(here, 'src', 'python', "%s.so" % ext.name), self.get_ext_fullpath(ext.name))


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

# Get the current version for the python package from the VERSION file
with open(path.join(here, 'VERSION'), encoding='utf-8') as f:
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
    },
    # tox.ini handles additional test dependencies
    tests_require=['tox'],
)
