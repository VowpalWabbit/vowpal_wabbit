default: vw
all: vw library_example java spanning_tree python

# CMake configs
ensure_cmake:
	mkdir -p build
	cd build; cmake ..

ensure_cmake_gcov:
	mkdir -p build
	cd build; cmake .. -DGCOV=On

ensure_cmake_profile:
	mkdir -p build
	cd build; cmake .. -DPROFILE=On

ensure_cmake_valgrind:
	mkdir -p build
	cd build; cmake .. -DVALGRIND_PROFILE=On

ensure_cmake_static:
	mkdir -p build
	cd build; cmake .. -DSTATIC_LINK_VW=On

# Build targets
spanning_tree_build:
	cd build; make -j$(shell cat ./build/nprocs.txt) spanning_tree

vw_build:
	cd build; make -j$(shell cat ./build/nprocs.txt) vw-bin

active_interactor_build:
	cd build; make -j$(shell cat ./build/nprocs.txt) active_interactor

library_example_build:
	cd build; make -j$(shell cat ./build/nprocs.txt) ezexample_predict ezexample_predict_threaded ezexample_train library_example test_search search_generate recommend gd_mf_weights

python_build:
	cd build; make -j$(shell cat ./build/nprocs.txt) pylibvw

java_build:
	cd build; make -j$(shell cat ./build/nprocs.txt) vw_jni

test_build:
	@echo "vw running test-suite..."
	cd build; make -j$(shell cat ./build/nprocs.txt) test_with_output

unit_test_build:
	cd build/test/unit_test; make -j$(shell cat ./build/nprocs.txt) vw-unit-test.out test

bigtests_build:
	cd build; make -j$(shell cat ./build/nprocs.txt) bigtests BIG_TEST_ARGS="$(MAKEFLAGS)"

install_build:
	cd build; make -j$(shell cat ./build/nprocs.txt) install

doc_build:
	cd build; make doc

# These can be invoked with [gcov, valgrind, profile, static]
spanning_tree_%: ensure_cmake_% spanning_tree_build ;
vw_%: ensure_cmake_% vw_build ;
active_interactor_%: ensure_cmake_% active_interactor_build ;
library_example_%: ensure_cmake_% library_example_build ;
python_%: ensure_cmake_% python_build ;
java_%: ensure_cmake_% java_build ;
test_%: ensure_cmake_% test_build ;
unit_test_%: ensure_cmake_% unit_test_build ;
bigtests_%: ensure_cmake_% bigtests_build ;
install_%: ensure_cmake_% install_build ;

# Normal build commands that use default configuration
spanning_tree: ensure_cmake spanning_tree_build ;
vw: ensure_cmake vw_build ;
active_interactor: ensure_cmake active_interactor_build ;
library_example: ensure_cmake library_example_build ;
python: ensure_cmake python_build ;
java: ensure_cmake java_build ;
test: ensure_cmake test_build ;
unit_test: ensure_cmake unit_test_build ;
bigtests: ensure_cmake bigtests_build ;
install: ensure_cmake install_build ;

clean:
	rm -rf build
