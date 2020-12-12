#!/bin/bash
set -e
set -x

pip install six

# https://github.com/actions/virtual-environments/issues/1811
brew uninstall openssl@1.0.2t

brew install cmake
brew install boost
brew install flatbuffers
