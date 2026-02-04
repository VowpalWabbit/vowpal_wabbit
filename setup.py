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

    if system == "Windows":
        global_options += [
            ("vcpkg-root=", None, "Path to vcpkg root. For Windows only"),
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

        # Don't build extensions again (CMake already did), but we need to
        # copy them to the build lib directory for packaging
        for ext in self.extensions:
            fullname = self.get_ext_fullname(ext.name)
            filename = self.get_ext_filename(fullname)

            # On Windows, find where CMake actually built the extension
            if system == "Windows":
                import glob

                config = "Debug" if self.distribution.debug else "Release"
                # CMake builds to build/temp.*/python/Release/ on Windows
                # Note: self.build_temp already includes the config directory
                pattern = os.path.join(self.build_temp, "python", config, filename)
                matches = glob.glob(pattern)
                if matches:
                    src = matches[0]
                else:
                    raise RuntimeError(f"Could not find built extension at {pattern}")
            else:
                # On Unix, CMake builds to lib_output_dir
                src = os.path.join(
                    here, os.path.dirname(self.get_ext_fullpath(ext.name)), filename
                )

            # Destination: where setuptools expects it for packaging
            dest = os.path.join(self.build_lib, filename)

            # Copy the extension
            self.copy_file(src, dest)

    def build_extension(self, ext):
        # CMake has already built the extension, skip normal build
        pass

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
            "-DVW_SIMD_INV_SQRT=Off",  # Use std::sqrt for reproducible results
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

        # Automatically detect pybind11 if available
        try:
            import pybind11

            pybind11_dir = pybind11.get_cmake_dir()
            cmake_args += ["-Dpybind11_DIR={}".format(pybind11_dir)]
        except (ImportError, AttributeError):
            # pybind11 not available or doesn't have get_cmake_dir
            pass

        # Read CMAKE_ARGS from environment variable if set
        if "CMAKE_ARGS" in os.environ:
            import shlex

            env_cmake_args = shlex.split(os.environ["CMAKE_ARGS"])
            cmake_args += env_cmake_args

        # Apply compiler flags for ARM64 Linux builds:
        # - armv8-a+crc: ARMv8.0 baseline with CRC32 extension (widely available)
        # - mno-outline-atomics: Prevents GCC from emitting LSE atomic instructions
        #   via libatomic calls, which can cause illegal instruction faults
        machine = platform.machine()
        if system == "Linux" and machine in ("aarch64", "arm64"):
            arm_flags = "-march=armv8-a+crc -mno-outline-atomics"
            cmake_args += [
                "-DCMAKE_CXX_FLAGS={}".format(arm_flags),
                "-DCMAKE_C_FLAGS={}".format(arm_flags),
            ]

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

        if system == "Windows":
            cmake_args += [
                "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=" + str(lib_output_dir),
                "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=" + str(lib_output_dir),
            ]

            if cmake_generator is None:
                # Check CMAKE_GENERATOR environment variable first
                cmake_generator = os.environ.get("CMAKE_GENERATOR")
                if cmake_generator is None:
                    cmake_generator = "Visual Studio 15 2017 Win64"

            build_args += ["--target", "pylibvw"]

            if self.distribution.vcpkg_root is not None:
                # add the vcpkg toolchain if its provided
                abs_vcpkg_path = os.path.abspath(self.distribution.vcpkg_root)
                vcpkg_toolchain = os.path.join(
                    abs_vcpkg_path, "scripts", "buildsystems", "vcpkg.cmake"
                )
                cmake_args += ["-DCMAKE_TOOLCHAIN_FILE=" + vcpkg_toolchain]

        else:
            cmake_args += [
                "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + str(lib_output_dir),
            ]
            build_args += [
                "--",
                "-j{}".format(multiprocessing.cpu_count()),
                # Build the pylibvw target
                "pylibvw",
            ]

        if cmake_generator is not None:
            cmake_args += ["-G", cmake_generator]

            if cmake_generator == "Visual Studio 16 2019":
                # The VS2019 generator now uses the -A option to select the toolchain's architecture
                cmake_args += ["-Ax64"]

        os.chdir(str(self.build_temp))
        self.spawn(["cmake"] + cmake_args + [str(here)])
        if not self.dry_run:
            self.spawn(["cmake", "--build", "."] + build_args)
        os.chdir(str(here))

        # On Windows, copy DLL next to the .pyd file (at root level)
        if system == "Windows" and "CONDA_PREFIX" in os.environ:
            import glob
            import shutil

            conda_bin = os.path.join(os.environ["CONDA_PREFIX"], "Library", "bin")
            print(f"Looking for runtime DLLs in: {conda_bin}")

            # Copy all DLLs that pylibvw might depend on (zlib, etc.)
            # Note: pybind11 is header-only and doesn't need DLLs
            dll_patterns_to_copy = ["zlib*.dll", "libzlib*.dll"]

            conda_dlls = []
            for pattern in dll_patterns_to_copy:
                dll_pattern = os.path.join(conda_bin, pattern)
                conda_dlls.extend(glob.glob(dll_pattern))

            print(f"Found conda DLLs: {[os.path.basename(d) for d in conda_dlls]}")

            if conda_dlls:
                # Copy DLLs to root (where .pyd is) for import to work
                for dll in conda_dlls:
                    dest = os.path.join(lib_output_dir, os.path.basename(dll))
                    print(f"Copying {dll} to {dest} (root level)")
                    shutil.copy2(dll, dest)

                # Also copy to vowpalwabbit directory for package_data
                vowpalwabbit_dir = os.path.join(lib_output_dir, "vowpalwabbit")
                if os.path.exists(vowpalwabbit_dir):
                    for dll in conda_dlls:
                        dest = os.path.join(vowpalwabbit_dir, os.path.basename(dll))
                        print(f"Copying {dll} to {dest} (package directory)")
                        shutil.copy2(dll, dest)
            else:
                print(f"Warning: No conda DLLs found in {conda_bin}")

            # Also copy all DLLs from build directory (VW core + any vcpkg dependencies)
            print(f"\nLooking for all DLLs in build directory...")
            config = "Debug" if self.distribution.debug else "Release"
            dll_patterns = [
                os.path.join(self.build_temp, config, "**", "*.dll"),
                os.path.join(self.build_temp, "**", "*.dll"),
            ]
            build_dlls = []
            for pattern in dll_patterns:
                found = glob.glob(pattern, recursive=True)
                build_dlls.extend(found)

            # Remove duplicates and filter out python DLLs
            seen = set()
            vw_dlls = []
            for dll in build_dlls:
                basename = os.path.basename(dll)
                # Skip python DLLs and duplicates
                if basename not in seen and not basename.startswith("python"):
                    seen.add(basename)
                    vw_dlls.append(dll)

            print(f"Found DLLs in build: {[os.path.basename(d) for d in vw_dlls]}")

            if vw_dlls:
                for dll in vw_dlls:
                    # Copy to root (where .pyd is)
                    dest = os.path.join(lib_output_dir, os.path.basename(dll))
                    print(f"Copying {dll} to {dest} (root level)")
                    shutil.copy2(dll, dest)

                    # Also copy to vowpalwabbit directory
                    vowpalwabbit_dir = os.path.join(lib_output_dir, "vowpalwabbit")
                    if os.path.exists(vowpalwabbit_dir):
                        dest = os.path.join(vowpalwabbit_dir, os.path.basename(dll))
                        print(f"Copying {dll} to {dest} (package directory)")
                        shutil.copy2(dll, dest)
            else:
                print(f"Warning: No VW DLLs found in build directory")

            print(f"\nDLL files at root:")
            for f in os.listdir(lib_output_dir):
                if f.endswith(".dll"):
                    print(f"  {f}")


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
    python_requires=">=3.10",
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
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
    ],
    keywords="fast machine learning online classification regression",
    package_dir={"": os.path.relpath(pkg_path)},
    packages=find_packages(where=pkg_path),
    package_data={"": ["*.dll"], "vowpalwabbit": ["*.dll"]},  # Include DLL files
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
)
