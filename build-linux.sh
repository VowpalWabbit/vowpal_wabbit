#!/bin/sh

cmake .
cmake --build . -- -j 8
cmake --build . --target RunTests
cmake --build . --target check