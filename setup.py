# -*- coding: utf-8 -*-
""" Vowpal Wabbit python setup module """

import os
import platform
import sys
import sysconfig
import subprocess
from codecs import open
from setuptools import setup, Extension, find_packages, Distribution
from setuptools.command.build_ext import build_ext
import multiprocessing

SYSTEM = platform.system()
PYTHON_VERSION = sys.version_info
REPO_ROOT_DIR = os.path.abspath(os.path.dirname(__file__))
PYTHON_PACKAGE_DIR = os.path.join(REPO_ROOT_DIR, "python")

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


class CustomDistribution(Distribution):
    global_options = Distribution.global_options

    global_options += [
        ("enable-boost-cmake", None, "Enable boost-cmake"),
        (
            "cmake-options=",
            None,
            "Additional semicolon-separated cmake setup options list",
        ),
        ("debug", None, "Debug build"),
    ]

    if SYSTEM == "Windows":
        global_options += [
            ("vcpkg-root=", None, "Path to vcpkg root. For Windows only"),
        ]

    def __init__(self, attrs=None):
        self.vcpkg_root = None
        self.enable_boost_cmake = None
        self.cmake_options = None
        self.cmake_generator = None
        self.debug = False
        Distribution.__init__(self, attrs)


class CMakeExtension(Extension):
    def __init__(self, name):
        # don't invoke the original build_ext for this special extension
        Extension.__init__(self, name, sources=[])


def get_ext_filename_without_platform_suffix(filename):
    from distutils.sysconfig import get_config_var

    ext_suffix = get_config_var("EXT_SUFFIX")
    name, ext = os.path.splitext(filename)

    if not ext_suffix:
        return filename

    if ext_suffix == ext:
        return filename

    ext_suffix = ext_suffix.replace(ext, "")
    idx = name.find(ext_suffix)

    if idx == -1:
        return filename
    else:
        return name[:idx] + ext


class BuildPyLibVWBindingsModule(build_ext):
    def get_ext_filename(self, ext_name):
        # don't append the extension suffix to the binary name
        # see https://stackoverflow.com/questions/38523941/change-cythons-naming-rules-for-so-files/40193040#40193040
        filename = build_ext.get_ext_filename(self, ext_name)
        return get_ext_filename_without_platform_suffix(filename)

    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        build_ext.run(self)

    def build_cmake(self, ext):
        # Ensure lib output directory is made
        lib_output_dir = os.path.join(REPO_ROOT_DIR, os.path.dirname(self.get_ext_fullpath(ext.name)))

        # example of cmake args
        config = "Debug" if self.distribution.debug else "Release"

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={lib_output_dir}",
            f"-DCMAKE_BUILD_TYPE={config}",
            "-DPY_VERSION=" + "{v[0]}.{v[1]}".format(v=PYTHON_VERSION),
            "-DBUILD_PYTHON=On",
            "-DBUILD_TESTS=Off",
            "-DWARNINGS=Off",
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
            cmake_args += [
                f"-DCMAKE_PREFIX_PATH={os.environ['CONDA_PREFIX']}",
                f"-DPython_INCLUDE_DIR={sysconfig.get_path('include')}",
            ]

        # Inject toolchain arguments for when vcpkg is specified.
        if self.distribution.vcpkg_root is not None:
            # add the vcpkg toolchain if its provided
            abs_vcpkg_path = os.path.abspath(self.distribution.vcpkg_root)
            vcpkg_toolchain = os.path.join(
                abs_vcpkg_path, "scripts", "buildsystems", "vcpkg.cmake"
            )
            cmake_args += ["-DCMAKE_TOOLCHAIN_FILE=" + vcpkg_toolchain]

        build_args = ["--target", "pylibvw"]

        # User can always override generator choice with the envvar. (CMake picks it up directly)
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")
        # If not on Windows use Ninja if it is available
        if SYSTEM != "Windows":
            if not cmake_generator:
                try:
                    # Try import it to see if the Python package is available
                    import ninja  # noqa: F401

                    cmake_args += ["-GNinja"]
                except ImportError:
                    pass
        else:
            # Single config generators are handled "normally"
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})

            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", PLAT_TO_CMAKE[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{config.upper()}={lib_output_dir}"
                ]
                build_args += ["--config", config]

        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            build_args += [f"-j{multiprocessing.cpu_count()}"]

        self.spawn(
            ["cmake", "-S", str(REPO_ROOT_DIR), "-B", str(self.build_temp)] + cmake_args
        )
        self.spawn(["cmake", "--build", str(self.build_temp)] + build_args)


# Get the long description from the README file
with open(os.path.join(PYTHON_PACKAGE_DIR, "README.rst"), encoding="utf-8") as f:
    long_description = f.read()

# Get the current version for the python package from the configure.ac file
config_path = os.path.join(REPO_ROOT_DIR, "version.txt")
with open(config_path, encoding="utf-8") as f:
    version = f.readline().strip()

try:
    current_git_hash = (
        subprocess.check_output(["git", "rev-parse", "--short", "HEAD"])
        .decode("ascii")
        .strip()
    )
    if current_git_hash:
        version = version + "+" + current_git_hash
except FileNotFoundError:
    pass
except subprocess.CalledProcessError:
    pass

setup(
    name="vowpalwabbit",
    version=version,
    python_requires=">=3.6",
    description="Vowpal Wabbit Python package",
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
    package_dir={"": os.path.relpath(PYTHON_PACKAGE_DIR)},
    packages=find_packages(where=PYTHON_PACKAGE_DIR),
    zip_safe=False,
    include_package_data=True,
    ext_modules=[CMakeExtension("pylibvw")],
    distclass=CustomDistribution,
    cmdclass={"build_ext": BuildPyLibVWBindingsModule},
)
