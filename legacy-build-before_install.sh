#!/bin/sh
sudo apt-get update -qq
sudo apt-get install -qq libboost-all-dev
sudo apt-get install maven
sudo pip install cpp-coveralls wheel

