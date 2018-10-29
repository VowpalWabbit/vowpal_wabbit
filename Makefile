ensure_cmake:
	mkdir -p build
	cd build; cmake ..

ensure_cmake_gcov:
	mkdir -p build
	cd build; cmake .. -DGCOV=On

default:	vw

all:	vw library_example java spanning_tree python

spanning_tree: ensure_cmake
	cd build; make -j$(cat nprocs.txt) spanning_tree

vw: ensure_cmake
	cd build; make -j$(cat nprocs.txt) vw-bin

vw_gcov: ensure_cmake_gcov vw

active_interactor: ensure_cmake
	cd build; make -j$(cat nprocs.txt) active_interactor

library_example: ensure_cmake
	cd build; make -j$(cat nprocs.txt) ezexample_predict ezexample_predict_threaded ezexample_train library_example test_search search_generate recommend gd_mf_weights

library_example_gcov: ensure_cmake_gcov library_example

python: ensure_cmake
	cd build; make -j$(cat nprocs.txt) pylibvw

java: ensure_cmake
	cd build; make -j$(cat nprocs.txt) vw_jni

test: ensure_cmake
	@echo "vw running test-suite..."
	cd build; make -j$(cat nprocs.txt) test_with_output

unit_test: ensure_cmake
	cd build/test/unit_test; make -j$(cat nprocs.txt) vw-unit-test.out test

test_gcov: ensure_cmake_gcov test

bigtests: ensure_cmake
	cd build; make -j$(cat nprocs.txt) bigtests BIG_TEST_ARGS="$(MAKEFLAGS)"

install: ensure_cmake
	cd build; make -j$(cat nprocs.txt) install

doc: ensure_cmake
	cd build; make doc

clean:
	rm -rf build
