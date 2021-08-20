#!/bin/bash
set -e
set -x

# This is causing a CI failure in the upgrade process. Remove it for now.
brew uninstall mongodb-community

brew update || brew update
brew upgrade
brew install cmake
brew install boost
brew install flatbuffers
