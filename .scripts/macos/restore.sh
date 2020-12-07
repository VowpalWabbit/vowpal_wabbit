#!/bin/bash
set -e
set -x

pip install six
brew uninstall openssl@1.0.2t
brew install cmake
brew install boost
brew install flatbuffers
