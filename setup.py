import os
import shutil
import subprocess
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext as _build_ext
from distutils.command.clean import clean as _clean

VERSION = None
with open("configure.ac") as f:
    for line in f:
        line = line.strip()
        if 'AC_INIT(' in line:
            VERSION = line.split(",")[1].replace("[", "").replace("]", "").strip()

if not VERSION:
    raise Exception("VowPal Wabbit version not found in '%s'" % CONFIG_H)


class VWBuildExt(_build_ext):
    def build_extension(self, ext):
        subprocess.check_call(["make", "python"])
        target_dir = os.path.dirname(self.get_ext_fullpath(ext.name))
        if not os.path.isdir(target_dir):
            os.makedirs(target_dir)
        shutil.copy(os.path.join("python", "%s.so" % ext.name), self.get_ext_fullpath(ext.name))


class VWClean(_clean):
    def run(self):
        _clean.run(self)
        subprocess.check_call(["make", "clean"])


pylibvw = Extension('pylibvw', sources=['python/pylibvw.cc'])

setup(
    name="pyvw",
    version=VERSION,
    url="https://github.com/JohnLangford/vowpal_wabbit",
    maintainer="trbs",
    maintainer_email="trbs@trbs.net",
    description="The Vowpal Wabbit (VW) project is a fast out-of-core learning system.",
    long_description="""PIP Installable version of Vowpal Wabbit.

You need to have boost (libboost-program-options-dev and libboost-python-dev) and python
development packages installed on your system for it to build correctly. See the Vowpal Wabbit
side for more information about building VW.

Since the pyvw wrapper bundled with Vowpal Wabbit links statically to libvw.a this package will
always build it's own library and (currently) cannot use the system installed libvw.so provided
by distribution packages.
""",
    package_dir={'': 'python'},
    py_modules=['pyvw'],
    ext_modules=[pylibvw],
    cmdclass={
        'build_ext': VWBuildExt,
        'clean': VWClean,
    },
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Topic :: Scientific/Engineering',
    ],
)
