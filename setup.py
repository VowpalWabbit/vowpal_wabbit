# -*- coding: utf-8 -*-
"""Vowpal Wabbit python setup module"""

import distutils.dir_util
import os
import platform
import sys
from codecs import open
from distutils.command.clean import clean as _clean
from distutils.sysconfig import get_python_inc
from setuptools import setup, Extension, find_packages, Distribution as _distribution
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
from setuptools.command.install_lib import install_lib as _install_lib
from shutil import rmtree
import multiprocessing

system = platform.system()
version_info = sys.version_info
here = os.path.abspath(os.path.dirname(__file__))
pkg_path = os.path.join(here, "python")


class Distribution(_distribution):
    global_options = _distribution.global_options

    global_options += [
        ("enable-boost-cmake", None, "Enable boost-cmake"),
        (
            "cmake-options=",
            None,
            "Additional semicolon-separated cmake setup options list",
        ),
        ("cmake-generator=", None, "CMake generator to use"),
        ("debug", None, "Debug build"),
    ]

    def __init__(self, attrs=None):
        self.vcpkg_root = None
        self.enable_boost_cmake = None
        self.cmake_options = None
        self.cmake_generator = None
        self.debug = False
        _distribution.__init__(self, attrs)


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
        lib_output_dir = os.path.join(
            here, os.path.dirname(self.get_ext_fullpath(ext.name))
        )
        distutils.dir_util.mkpath(lib_output_dir)

        # example of cmake args
        config = "Debug" if self.distribution.debug else "Release"

        cmake_args = [
            "-DCMAKE_BUILD_TYPE=" + config,
            "-DPY_VERSION=" + "{v[0]}.{v[1]}".format(v=version_info),
            "-DBUILD_PYTHON=On",
            "-DBUILD_TESTING=Off",
            "-DWARNINGS=Off",
            "-DVW_FEAT_CB_GRAPH_FEEDBACK=On",
            "-DSTD_INV_SQRT=On",
        ]

        # Allow BOOST_PY_VERSION_SUFFIX to be overridden via environment variable
        # (used by cibuildwheel for manylinux/macOS where only generic python3 is available)
        if "BOOST_PY_VERSION_SUFFIX" in os.environ:
            cmake_args += [
                "-DBOOST_PY_VERSION_SUFFIX=" + os.environ["BOOST_PY_VERSION_SUFFIX"]
            ]

        # This doesn't work as expected for Python3.6 and 3.7 on Windows.
        # See bug: https://bugs.python.org/issue39825
        if system == "Windows" and sys.version_info.minor < 8:
            from distutils import sysconfig as distutils_sysconfig

            required_shared_lib_suffix = distutils_sysconfig.get_config_var(
                "EXT_SUFFIX"
            )
        else:
            import sysconfig

            required_shared_lib_suffix = sysconfig.get_config_var("EXT_SUFFIX")

        if required_shared_lib_suffix is not None:
            cmake_args += [
                "-DVW_PYTHON_SHARED_LIB_SUFFIX={}".format(required_shared_lib_suffix)
            ]

        if self.distribution.enable_boost_cmake is None:
            # Add this flag as default since testing indicates its safe.
            # But add a way to disable it in case it becomes a problem
            cmake_args += ["-DBoost_NO_BOOST_CMAKE=ON"]

        if self.distribution.cmake_options is not None:
            argslist = self.distribution.cmake_options.split(";")
            cmake_args += argslist

        # If we are being installed in a conda environment then use the dependencies from there.
        if "CONDA_PREFIX" in os.environ:
            cmake_args.append(
                "-DCMAKE_PREFIX_PATH={}".format(os.environ["CONDA_PREFIX"])
            )
            cmake_args.append("-DPython_INCLUDE_DIR={}".format(get_python_inc()))

        # example of build args
        build_args = ["--config", config]

        cmake_generator = self.distribution.cmake_generator

        # Auto-detect vcpkg in ext_libs/vcpkg if it exists and is bootstrapped
        vcpkg_root = self.distribution.vcpkg_root
        if vcpkg_root is None:
            potential_vcpkg = os.path.join(here, "ext_libs", "vcpkg")
            vcpkg_exe = os.path.join(
                potential_vcpkg, "vcpkg.exe" if system == "Windows" else "vcpkg"
            )
            # Only use vcpkg if it's been bootstrapped (vcpkg executable exists)
            if os.path.exists(vcpkg_exe):
                vcpkg_root = potential_vcpkg

        if vcpkg_root is not None:
            # add the vcpkg toolchain if its provided
            abs_vcpkg_path = os.path.abspath(vcpkg_root)
            vcpkg_toolchain = os.path.join(
                abs_vcpkg_path, "scripts", "buildsystems", "vcpkg.cmake"
            )
            if os.path.exists(vcpkg_toolchain):
                cmake_args += ["-DCMAKE_TOOLCHAIN_FILE=" + vcpkg_toolchain]
                # When using vcpkg, explicitly set Python paths to avoid vcpkg's Python
                # interfering with the build environment's Python (especially on Windows)
                import sysconfig

                cmake_args += [
                    "-DPython_EXECUTABLE=" + sys.executable,
                    "-DPython_FIND_STRATEGY=LOCATION",  # Prefer Python_EXECUTABLE over registry/environment
                ]
                python_root = sysconfig.get_config_var("prefix")
                if python_root:
                    cmake_args += ["-DPython_ROOT_DIR=" + python_root]

        if system == "Windows":
            cmake_args += [
                "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=" + str(lib_output_dir),
                "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=" + str(lib_output_dir),
            ]
            if cmake_generator is None:
                cmake_generator = "Visual Studio 17 2022"

        else:
            cmake_args += [
                "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + str(lib_output_dir),
            ]
            if cmake_generator is None:
                cmake_generator = "Ninja"

        if cmake_generator is not None:
            cmake_args += ["-G", cmake_generator]
            if cmake_generator == "Visual Studio 16 2019":
                # The VS2019 generator now uses the -A option to select the toolchain's architecture
                cmake_args += ["-Ax64"]

        # Build the pylibvw target
        build_args += ["--target", "pylibvw"]

        os.chdir(str(self.build_temp))
        self.spawn(["cmake"] + cmake_args + [str(here)])
        if not self.dry_run:
            self.spawn(["cmake", "--build", "."] + build_args)
        os.chdir(str(here))


class Clean(_clean):
    """Clean up after building python package directories"""

    def run(self):
        rmtree(os.path.join(here, "dist"), ignore_errors=True)
        rmtree(os.path.join(here, "build"), ignore_errors=True)
        rmtree(os.path.join(here, "vowpalwabbit.egg-info"), ignore_errors=True)
        _clean.run(self)


class Sdist(_sdist):
    def run(self):
        _sdist.run(self)


class InstallLib(_install_lib):
    def build(self):
        _install_lib.build(self)


# Get the long description from the README file
with open(os.path.join(pkg_path, "README.rst"), encoding="utf-8") as f:
    long_description = f.read()

# Get the current version for the python package from the configure.ac file
config_path = os.path.join(here, "version.txt")
with open(config_path, encoding="utf-8") as f:
    version = f.readline().strip()

setup(
    name="vowpalwabbit",
    version=version,
    python_requires=">=3.6",
    description="Vowpal Wabbit Python package",
    long_description_content_type="text/x-rst",
    long_description=long_description,
    url="https://github.com/VowpalWabbit/vowpal_wabbit",
    author="Scott Graham",
    author_email="scott.d.graham@gmail.com",
    license="BSD 3-Clause License",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering",
        "Topic :: Scientific/Engineering :: Information Analysis",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: Python :: 3",
    ],
    keywords="fast machine learning online classification regression",
    package_dir={"": os.path.relpath(pkg_path)},
    packages=find_packages(where=pkg_path),
    platforms="any",
    zip_safe=False,
    include_package_data=True,
    ext_modules=[CMakeExtension("pylibvw")],
    distclass=Distribution,
    cmdclass={
        "build_ext": BuildPyLibVWBindingsModule,
        "clean": Clean,
        "sdist": Sdist,
        "install_lib": InstallLib,
    },
    install_requires=[
        "numpy",
        "scipy",
        "scikit-learn",
        "pandas",
    ],
)
