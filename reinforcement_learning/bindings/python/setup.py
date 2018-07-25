import glob
import setuptools

with open("README.md", "r") as fh:
	long_description = fh.read()

extension_module = setuptools.Extension(
	'rlinference._rlinference',
	sources = glob.glob('*.cc'),
	library_dirs = [],
	include_dirs = ['../../include/'],
	libraries = ['pthread', 'dl'],
	extra_compile_args = ['-std=c++11'],
	extra_objects = ['../../rlclientlib/librlclient.a', '/usr/local/lib/libcpprest.a', '../../../vowpalwabbit/libvw.a', '../../../vowpalwabbit/liballreduce.a', \
			 '/home/ataymano/boost_1_58_0/boost_output/lib/libboost_system.a', '/home/ataymano/boost_1_58_0/boost_output/lib/libboost_program_options.a', \
			 '/home/ataymano/ssl/lib/libssl.a', '/home/ataymano/ssl/lib/libcrypto.a', '/home/ataymano/zlib/lib/libz.a']
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

