import glob
import setuptools

with open("README.md", "r") as fh:
	long_description = fh.read()

external_deps_dir = '/home/ataymano/restore_e2e_1/'

extension_module = setuptools.Extension(
	'rlinference._rlinference',
	sources = glob.glob('*.cc'),
	library_dirs = [],
	include_dirs = ['../../include/'],
	libraries = ['pthread', 'dl'],
	extra_compile_args = ['-std=c++11'],
	extra_objects = ['../../rlclientlib/librlclient.a', external_deps_dir + 'libcpprest.a', '../../../vowpalwabbit/libvw.a', '../../../vowpalwabbit/liballreduce.a', \
			 external_deps_dir + 'libboost_system.a', external_deps_dir + 'libboost_program_options.a', \
			 external_deps_dir + 'libssl.a', external_deps_dir + 'libcrypto.a', external_deps_dir + 'libz.a']
)

setuptools.setup(
	name = "rlinference",
	version = "0.0.3",
	author = "Microsoft",
	author_email = "email?",
	description = "Python binding for reinforcement learning client library",
	long_description = long_description,
	url = "url?",
	ext_modules = [extension_module],
	py_modules = ['rlinference.py'],
	packages = setuptools.find_packages(),
	classifiers = (
		"Programming Language :: Python :: 3",
		"Operating System :: OS Independent"
	)
)

