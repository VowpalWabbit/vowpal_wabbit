#!/bin/bash
set -e
set -x

# Run Java build and test
cd $1
mvn clean test -f java/pom.xml
