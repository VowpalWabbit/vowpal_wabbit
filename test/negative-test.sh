#!/usr/bin/env bash

# this script runs a command and ensures that its exit code is not zero.

if $@; then
    exit 0
else
    exit 1
fi
