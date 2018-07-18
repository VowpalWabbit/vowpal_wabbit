# -*- coding: utf-8 -*-
""" Vowpal Wabbit python setup module """

import platform
import re
import subprocess
import sys
import sysconfig
from codecs import open
from ctypes.util import find_library
from distutils.command.clean import clean as _clean
from os import environ, makedirs, path, remove, walk
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
from setuptools.command.test import test as _test
from setuptools.command.install_lib import install_lib as _install_lib
from shutil import copy, copytree, rmtree


system = platform.system()
version_info = sys.version_info
here = path.abspath(path.dirname(__file__))
pylibvw = Extension('pylibvw', sources=['python/pylibvw.cc'])


def find_boost():
    """ Find correct boost-python library information """
    # find_library() has a tricky platform-dependent search behaviour
    # it is not easy to instruct it to do search in a particular location on Linux at least
    # skip find_library checks if python comes from conda 
    # rely on conda to set up libboost_python.so link to what has been actually installed

    prefix = 'boost_python'
    if environ.get('CONDA_PREFIX') is not None:
        environ['DYLD_LIBRARY_PATH'] = sysconfig.get_config_var('LIBDIR')
        options = ['']
    elif system == 'Linux':
        options = ['-py{v[0]}{v[1]}'.format(v=version_info),
                   '-py{v[0]}'.format(v=version_info),
                   '{v[0]}'.format(v=version_info),
                   '']
    elif system == 'Darwin':
        options = ['{v[0]}{v[1]}-mt'.format(v=version_info),
                   '{v[0]}-mt'.format(v=version_info),
                   '{v[0]}{v[1]}'.format(v=version_info),
                   '{v[0]}'.format(v=version_info),
                   '-mt',
                   '']
    elif system == 'Cygwin':
        options = ['{v[0]}-mt'.format(v=version_info),
                   '-mt']
    else:
        raise Exception('Building on this system is not currently supported')

    for option in options:
        boost_lib = '{pre}{opt}'.format(pre=prefix, opt=option)
        if find_library(boost_lib):
            break
    else:
        raise Exception('Could not find boost python library')

    return boost_lib


def prep():
    """ Prepare source directories for building extension """

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
        copytree(path.join(here, '..', 'rapidjson'), path.join(here, 'src', 'rapidjson'))
        copytree(path.join(here, '..', 'explore'), path.join(here, 'src', 'explore'))

        # add folders necessary to run 'make python'
        for folder in ['library', 'vowpalwabbit']:
            copytree(path.join(here, '..', folder), path.join(here, 'src', folder))
            subprocess.check_call(['make', 'clean'], cwd=path.join(here, 'src', folder))


class Clean(_clean):
    """ Clean up after building python package directories """
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
        # run prep if needed
        prep()
        _sdist.run(self)


class VWBuildExt(_build_ext):
    """ Build pylibvw.so and install it as a python extension """
    def build_extension(self, ext):
        prep()
        target_dir = path.dirname(self.get_ext_fullpath(ext.name))
        if not path.isdir(target_dir):
            makedirs(target_dir)
        if system == 'Windows':
            sys_version = '{v[0]}{v[1]}'.format(v=version_info)
            if sys_version in ['27', '35', '36']:
                copy(path.join(here, 'bin', 'pyvw{}.dll'.format(sys_version)), self.get_ext_fullpath(ext.name))
            else:
                raise Exception('Pre-built vw/python library for Windows is not supported for this python version')
        else:
            py_config_path = 'python-config'
            projectbase = sysconfig.get_config_var('projectbase')
            if projectbase is not None:
                configs = ['python{v[0]}.{v[1]}-config'.format(v=version_info),
                           'python{v[0]}-config'.format(v=version_info),
                           'python-config']
                for config in configs:
                    py_config_path = path.join(projectbase, config)
                    if path.exists(py_config_path):
                        environ['PYTHON_CONFIG'] = py_config_path
                        break

            environ['PYTHON_VERSION'] = '{v[0]}.{v[1]}'.format(v=version_info)
            environ['PYTHON_LIBS'] = '-l {}'.format(find_boost())
            environ['PYTHON_INCLUDE'] = subprocess.check_output([py_config_path, '--includes']).decode("utf-8").strip()
            environ['PYTHON_LDFLAGS'] = subprocess.check_output([py_config_path, '--ldflags']).decode("utf-8").strip()

            if system == 'Darwin':
                # Can't find a way around this hack, pulling LDFLAGS from sysconfig does not work in all cases
                environ['PYTHON_LDFLAGS'] = re.sub(',-stack_size,[0-9]*', '', environ['PYTHON_LDFLAGS'])

            # Use boost library install by Anaconda
            conda_base = environ.get('CONDA_PREFIX')
            if conda_base is not None:
                environ['USER_BOOST_INCLUDE'] = '-I {b}/include/boost -I {b}/include'.format(b=conda_base)
                environ['USER_BOOST_LIBRARY'] = '-L {b}/lib'.format(b=conda_base)

            subprocess.check_call(['make', 'python'], cwd=path.join(here, 'src'), env=environ)
            ext_suffix = 'so' if not system == 'Cygwin' else 'dll'
            copy(path.join(here, 'src', 'python', '{name}.{suffix}'.format(name=ext.name, suffix=ext_suffix)),
                 self.get_ext_fullpath(ext.name))


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

# Get the current version for the python package from the configure.ac file
version = '0.0.0'
for config_path in [path.join(here, '..', 'configure.ac'), path.join(here, 'src', 'configure.ac')]:
    try:
        with open(config_path, encoding='utf-8') as f:
            line = f.readline().strip()
        version = line.split(',')[1].strip(' []')
    except IOError:
        continue

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
