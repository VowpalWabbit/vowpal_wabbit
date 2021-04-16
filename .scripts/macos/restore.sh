#!/bin/bash
set -e
set -x

pip install six

brew update || brew update
brew upgrade
brew install cmake
brew install boost
brew install flatbuffers
