#!/usr/bin/env bash

# this script runs a command and ensures that its exit code is not zero.

if $@; then
    exit 1
else
    exit 0
fi
