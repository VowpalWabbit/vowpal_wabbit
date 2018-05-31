#!/bin/bash
coveralls --exclude lib --exclude tests --gcov-options '\-lp'
