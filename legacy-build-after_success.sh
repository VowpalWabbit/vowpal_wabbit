#!/bin/sh
coveralls --exclude lib --exclude tests --gcov-options '\-lp'
