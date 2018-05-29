#!/bin/sh
export JAVA_HOME=/usr/lib/jvm/java-8-oracle

cmake .
cmake --build . -- -j 8
cmake --build . --target RunTests
cmake --build . --target check
