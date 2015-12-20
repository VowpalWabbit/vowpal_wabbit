# Makefile for big_tests

SHELL = bash

# suppress built-in rules, to speed things up
MAKEFLAGS=-r
# disable implicit suffixes, to speed things up
.SUFFIXES:
# keep all intermediate files
.SECONDARY:
.ONESHELL:
.SILENT:

.PHONY:	test expected pretend run clean spotless

TOP_MK_DIR := $(patsubst %/,%,$(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
ARCH ?= $(subst /,_,$(shell uname -o)_$(shell uname -i))
stageDir := $(TOP_MK_DIR)/$(ARCH)
dataDir := $(TOP_MK_DIR)/dataSets
expDir := $(TOP_MK_DIR)/expected
allTests := $(patsubst %/,%,$(shell ls $(expDir)))

# the ?= allows the user to override the executable from the command line, as in
# make EXEC=myProgram all
EXEC ?= $(TOP_MK_DIR)/../vowpalwabbit/vw
DIFF ?= diff

default: test
all:	test

# details of individual tests come from here
include dataSets.mk
include testDetails.mk

stub:
	echo $(allTests)

help:
	cat $(TOP_MK_DIR)/HOWTO.txt

test:
	@for aTest in $(allTests); do
		make $$aTest.test || exit
	done
	@echo "test suite finished"

# use this target to run without doing any diffs
run:
	@for aTest in $(allTests); do
		make $$aTest.run || exit 1
	done
	echo "'make run' finished"

expected:
	@for aTest in $(allTests); do
		make $$aTest.expected || exit
	done
	echo "'make expected' finished"

clean:
	-rm -r $(stageDir)

eraseData:
	-rm -r $(dataDir)/

pretend:
	@for aTest in $(allTests); do
		make $$aTest.pretend || exit
	done
	echo "'make pretend' finished"


%.test:	%.startNotice %.run %.compare %.endNotice ;

%.startNotice:
	@echo "Starting test $*"

%.endNotice:
	@echo "Finished test $*"

$(stageDir)/%.dir:
	@mkdir -p $@

%.run:	$(stageDir)/%.dir $(stageDir)/%.dir/done ;

$(stageDir)/%.dir/done:	$(dataDir)/$(%.inData)
	@if [ -n "$($*.deps)" ] ; then
		echo "test $* has dependencies: $($*.deps)"
		$(MAKE) $($*.deps)
		# ToDo: stop if deps fail
		echo "finished dependencies; proceeding with test $*"
	fi
	cd $(stageDir)/$*.dir
	exec=$(EXEC)
	if [ -n  "$($*.exec)" ] ; then exec="$($*.exec)" ; fi
	echo "$$exec $($*.params) $(EP) > out 2> err"
	$$exec $($*.params) $(EP) > out 2> err
	touch $@

# first diff all the results, then set exit status if non-empty
%.compare:	$(stageDir)/%.dir $(expDir)/%/out $(expDir)/%/err
	@cd $(stageDir)/$*.dir
	rm -f diffs
	for FILE in out err $($*.otherOutputs) ; do
		echo "$(DIFF) -b $$FILE $(expDir)/$*/$$FILE"
		$(DIFF) -b $$FILE $(expDir)/$*/$$FILE |& tee -a diffs
	done
	if [ -s diffs ] ; then
		echo "Test $* failed due to diffs." ;\
		exit 1
	fi

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
