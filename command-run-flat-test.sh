#!/bin/bash
mkdir build/utl/flatbuffer/fb-test
cp test/test-sets/0001.dat build/utl/flatbuffer/fb-test
# cd build/utl/flatbuffer/fb-test
./build/utl/flatbuffer/to_flatbuff -d build/utl/flatbuffer/fb-test/0001.dat --fb_out build/utl/flatbuffer/fb-test/0001.fb