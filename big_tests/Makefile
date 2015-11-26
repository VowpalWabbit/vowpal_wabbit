# Makefile for big_tests

SHELL = bash

# suppress built-in rules, to speed things up
MAKEFLAGS=-r

# disable implicit suffixes, to speed things up
.SUFFIXES:
# keep all intermediate files
.SECONDARY:
.ONESHELL:
.PHONY:	test expected pretend run clean spotless

TOP_MK_DIR := $(patsubst %/,%,$(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
ARCH := $(subst /,_,$(shell uname -o)_$(shell uname -i))
stageDir := $(TOP_MK_DIR)/$(ARCH)
dataDir := $(TOP_MK_DIR)/dataSets
expDir := $(TOP_MK_DIR)/expected

VW = $(TOP_MK_DIR)/../vowpalwabbit/vw
DIFF = diff

default: test
all:	test

# details of individual tests come from here
include dataSets.mk
include testDetails.mk

test:
	@for aTest in $(allTests); do
		make $$aTest.test || exit
	done
	@echo "made test"

# use this target to run without doing any diffs
run:
	@for aTest in $(allTests); do
		make $$aTest.run || exit 1
	done
	echo "made run"

expected:
	@for aTest in $(allTests); do
		make $$aTest.expected || exit
	done
	echo "made expected"

clean:
	-rm -r $(stageDir)

cleandata:
	-rm -r $(dataDir)/

pretend:
	@for aTest in $(allTests); do
		make $$aTest.pretend || exit
	done
	echo "made pretend"


%.test:	%.startNotice %.run %.compare %.endNotice
	@echo

%.startNotice:
	@echo "Starting test $*"

%.endNotice:
	@echo "Finished test $*"

$(stageDir)/%.dir:
	@mkdir -p $@

%.run:	%.deps $(stageDir)/%.dir $(stageDir)/%.dir/done
	@echo

$(stageDir)/%.dir/done:	$(dataDir)/$(%.inData)
	cd $(stageDir)/$*.dir
	if [ -n  "$($*.exec)" ] ; then
		$($*.exec) $($*.params) > out 2> err
	else
		$(VW) $($*.params) > out 2> err
	fi
	@touch $@

# first diff all the results, then set exit status if non-empty
%.compare:	$(stageDir)/%.dir $(stageDir)/%.dir/diffs
	@echo

$(stageDir)/%.dir/diffs:	$(expDir)/%/out $(expDir)/%/err
	@cd $(stageDir)/$*.dir
	rm -f diffs
	for FILE in out err $($*.otherOutputs) ; do
		echo "$(DIFF) -b $$FILE $(expDir)/$*/$$FILE"
		$(DIFF) -b $$FILE $(expDir)/$*/$$FILE |& tee -a diffs
	done
	if [ -s diffs ] ; then (echo "Check failed for test $*"; exit 1) ; fi

# update expected files from current target files
# $*.out and $*.err are always added as expected files
# copy files over only if they are different or absent, to avoid triggering dependencies
%.expected: %.run
	@cd $(stageDir)/$*.dir
	mkdir -p $(expDir)/$*/
	for FILE in out err $($*.otherOutputs) ; do
		$(DIFF) -qbr $$FILE $(expDir)/$*/$$FILE \
		|| (echo "copying $$FILE to $(expDir)/$*/" ; cp $$FILE $(expDir)/$*/ )
	done

# create targets from expected files, to pretend that the module succeeded
%.pretend:	$(stageDir)/%.dir $(expDir)/%/out $(expDir)/%/err
	@cd $(stageDir)/$*.dir
	for FILE in out err $($*.otherOutputs) ; do
		echo "copying $(expDir)/$*/$$FILE to $(stageDir)/$*.dir/"
		cp $(expDir)/$*/$$FILE .
	done

%.clean:
	-rm -rf $(stageDir)/$*
