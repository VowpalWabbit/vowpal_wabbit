# -*- coding: utf-8 -*-
""" Vowpal Wabbit python setup module """

import distutils.dir_util
import os
import platform
import sys
import sysconfig
import subprocess
from codecs import open
from distutils.command.clean import clean as _clean
from setuptools import setup, Extension, find_packages, Distribution as _distribution
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
from setuptools.command.install_lib import install_lib as _install_lib
from shutil import rmtree
import multiprocessing

system = platform.system()
version_info = sys.version_info
here = os.path.abspath(os.path.dirname(__file__))
pkg_path = os.path.join(here, 'python')

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    'win32': 'Win32',
    'win-amd64': 'x64',
    'win-arm32': 'ARM',
    'win-arm64': 'ARM64',
}

class Distribution(_distribution):
    global_options = _distribution.global_options

    global_options += [
        ('enable-boost-cmake', None, 'Enable boost-cmake'),
        ('cmake-options=', None, 'Additional semicolon-separated cmake setup options list'),
        ('debug', None, 'Debug build'),
        ('vcpkg-root=', None, 'Path to vcpkg root'),
    ]

    def __init__(self, attrs=None):
        self.vcpkg_root = None
        self.enable_boost_cmake = None
        self.cmake_options = None
        self.debug = False
        _distribution.__init__(self, attrs)


class CMakeExtension(Extension):
    def __init__(self, name):
        # don't invoke the original build_ext for this special extension
        Extension.__init__(self, name, sources=[])

def get_ext_filename_without_platform_suffix(filename):
    ext_suffix = sysconfig.get_config_var('EXT_SUFFIX')
    name, ext = os.path.splitext(filename)

    if not ext_suffix:
        return filename

    if ext_suffix == ext:
        return filename

    ext_suffix = ext_suffix.replace(ext, '')
    idx = name.find(ext_suffix)

    if idx == -1:
        return filename
    else:
        return name[:idx] + ext

class BuildPyLibVWBindingsModule(_build_ext):
    def get_ext_filename(self, ext_name):
        # don't append the extension suffix to the binary name
        # see https://stackoverflow.com/questions/38523941/change-cythons-naming-rules-for-so-files/40193040#40193040
        filename = _build_ext.get_ext_filename(self, ext_name)
        return get_ext_filename_without_platform_suffix(filename)

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
        config = 'Debug' if self.distribution.debug else 'Release'

        cmake_args = [
            '-DCMAKE_BUILD_TYPE=' + config,
            '-DPY_VERSION=' + '{v[0]}.{v[1]}'.format(v=version_info),
            '-DBUILD_PYTHON=On',
            '-DBUILD_TESTS=Off',
            '-DWARNINGS=Off'
        ]
        if self.distribution.enable_boost_cmake is None:
            # Add this flag as default since testing indicates its safe.
            # But add a way to disable it in case it becomes a problem
            cmake_args += [
                '-DBoost_NO_BOOST_CMAKE=ON'
            ]

        if self.distribution.cmake_options is not None:
            argslist = self.distribution.cmake_options.split(';')
            cmake_args += argslist

        # If we are being installed in a conda environment then use the dependencies from there.
        if 'CONDA_PREFIX' in os.environ:
                cmake_args.append('-DCMAKE_PREFIX_PATH={}'.format(os.environ['CONDA_PREFIX']))
                cmake_args.append('-DPython_INCLUDE_DIR={}'.format(sysconfig.get_path('include')))

        # Inject toolchain arguments for when vcpkg is specified.
        if self.distribution.vcpkg_root is not None:
            # add the vcpkg toolchain if its provided
            abs_vcpkg_path = os.path.abspath(self.distribution.vcpkg_root)
            vcpkg_toolchain = os.path.join(
                abs_vcpkg_path,
                'scripts',
                'buildsystems',
                'vcpkg.cmake'
            )
            cmake_args += ['-DCMAKE_TOOLCHAIN_FILE=' + vcpkg_toolchain]

        build_args = ['--target', 'pylibvw']

        # If not on Windows use Ninja if it is available
        if system == 'Windows':
            # User can always override generator choice with the envvar. (CMake picks it up directly)
            cmake_generator = os.environ.get('CMAKE_GENERATOR', '')

            # Single config generators are handled 'normally'
            single_config = any(x in cmake_generator for x in {'NMake', 'Ninja'})

            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {'ARM', 'Win64'})

            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ['-A', PLAT_TO_CMAKE[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    f'-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG={lib_output_dir}'
                    f'-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE={lib_output_dir}'
                ]
                build_args += ['--config', config]
            else:
                cmake_args += [f'-DCMAKE_RUNTIME_OUTPUT_DIRECTORY={lib_output_dir}']
        else:
            # We're making an assumption here that non-windows platforms use single_config generators
            cmake_args += [f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={lib_output_dir}']


        if 'CMAKE_BUILD_PARALLEL_LEVEL' not in os.environ:
            build_args += ['--parallel', str(multiprocessing.cpu_count())]

        self.spawn(
            ['cmake', '-S', str(here), '-B', str(self.build_temp)] + cmake_args
        )
        self.spawn(['cmake', '--build', str(self.build_temp)] + build_args)

class Clean(_clean):
    """ Clean up after building python package directories """
    def run(self):
        rmtree(os.path.join(here, 'dist'), ignore_errors=True)
        rmtree(os.path.join(here, 'build'), ignore_errors=True)
        rmtree(os.path.join(here, 'vowpalwabbit.egg-info'), ignore_errors=True)
        _clean.run(self)


class Sdist(_sdist):
    def run(self):
        _sdist.run(self)

class InstallLib(_install_lib):
    def build(self):
        _install_lib.build(self)

# Get the long description from the README file
with open(os.path.join(pkg_path, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

# Get the current version for the python package from the configure.ac file
config_path = os.path.join(here, 'version.txt')
with open(config_path, encoding='utf-8') as f:
    version = f.readline().strip()

try:
    current_git_hash = (
        subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'])
        .decode('ascii')
        .strip()
    )
    if current_git_hash:
        version = version + "+" + current_git_hash
except FileNotFoundError as e:
    pass
except subprocess.CalledProcessError as e:
    pass

setup(
    name='vowpalwabbit',
    version=version,
    python_requires='>=3.6',
    description='Vowpal Wabbit Python package',
    long_description=long_description,
    url='https://github.com/VowpalWabbit/vowpal_wabbit',
    author='Scott Graham',
    author_email='scott.d.graham@gmail.com',
    license='BSD 3-Clause License',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Information Analysis',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 3',
    ],
    keywords='fast machine learning online classification regression',
    package_dir={'' : os.path.relpath(pkg_path)},
    packages=find_packages(where=pkg_path),
    platforms='any',
    zip_safe=False,
    include_package_data=True,
    ext_modules=[CMakeExtension('pylibvw')],
    distclass=Distribution,
    cmdclass={
        'build_ext': BuildPyLibVWBindingsModule,
        'clean': Clean,
        'sdist': Sdist,
        'install_lib': InstallLib
    },
)
