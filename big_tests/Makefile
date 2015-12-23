# Makefile for big_tests

SHELL = bash

# suppress built-in rules, to speed things up
MAKEFLAGS+=r
# disable implicit suffixes, to speed things up
.SUFFIXES:
# keep all intermediate files
.SECONDARY:
.SECONDEXPANSION:
# .ONESHELL doesn't work with make < 3.82!
# .ONESHELL:

# comment out for debugging
.SILENT:

# can't compare before running; can't prep data before downloading; etc.
.NOTPARALLEL:

.PHONY:	default all stub help valid expected pretend run clean spotless %.valid %.expected %.run %.clean %.deps %.depsWrapper %.depsStart %.depsEnd %.stub

TOP_MK_DIR := $(patsubst %/,%,$(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
ARCH ?= $(subst /,_,$(shell uname -o)_$(shell uname -i))
# can't use the architecture as directory name, cuz that produces different expected files on different platforms
# stageDir := $(ARCH)
stageDir := runs
dataDir := dataSets
expDir := expected
allTests := $(patsubst %/,%,$(shell ls $(expDir)))

# the ?= allows the user to override the executable from the command line, as in
# make EXEC=myProgram all
# alas, $(realpath ...) doesn't seem to work here on make 3.81 for some reason
EXEC ?= ../vowpalwabbit/vw
DIFF ?= diff -b

all:	valid ;

.DEFAULT:
	@echo "No rules found for $@ in $(MAKEFILE_LIST)"

# details of individual tests come from here
include dataSets.mk
include testDetails.mk

# for debugging
%.stub:	
	echo $($*.deps)
	echo $(stageDir)/$*.dir/done
	echo $(addsuffix .valid,$($*.deps))

# for debugging
stub:	
	echo $(MAKEFLAGS)
	echo $(EXEC)
	echo $(TOP_MK_DIR)
	echo $(VPATH)

help:
	cat $(TOP_MK_DIR)/README.txt

validTargets := $(addsuffix .valid,$(allTests))
valid:	$(validTargets)
	@echo "test suite finished"

# use this target to run without doing any diffs
runTargets := $(addsuffix .run,$(allTests))
run:	$(runTargets)
	@echo "'make run' finished"

expectedTargets := $(addsuffix .expected,$(allTests))
expected:	$(expectedTargets)
	@echo "'make expected' finished"

pretendTargets := $(addsuffix .pretend,$(allTests))
pretend:	$(pretendTargets)
	@echo "'make pretend' finished"

clean:
	-rm -r $(stageDir)
	@echo "'make clean' finished"

%.dir:
	@mkdir -p $@

%.valid:	%.run %.compare ;

%.run:	%.startNotice $(stageDir)/%.dir/done %.endNotice ;

%.startNotice:
	@echo "Starting to run test $*"

%.endNotice:
	@echo "Finished running test $*"

$(dataDir) $(stageDir):
	@mkdir -p $@

%.depsWrapper:	%.depsStart $$(%.deps) %.depsEnd ;

%.depsStart:
	@if [ -n "$($*.deps)" ] ; then \
		echo "test $* has dependencies: $($*.deps)" ;\
	fi

%.depsEnd:
	@if [ -n "$($*.deps)" ] ; then \
		echo "dependencies satisfied; proceeding with test $*" ;\
	fi

# $(stageDir)/%.dir/done:	$(stageDir)/%.dir $$(%.deps) $$(%.inData)
$(stageDir)/%.dir/done:	$(stageDir)/%.dir $$(%.inData)
	exec=$(EXEC) ;\
	if [ -n "$($*.exec)" ] ; then exec="$($*.exec)" ; fi ;\
	echo "$$exec $($*.params) $(EP) > $(stageDir)/$*.dir/out 2> $(stageDir)/$*.dir/err" ;\
	$$exec $($*.params) $(EP) > $(stageDir)/$*.dir/out 2> $(stageDir)/$*.dir/err ;\
	touch $@

# first diff all the results, then set exit status if non-empty
%.compare:	$(stageDir)/%.dir $(expDir)/%/out $(expDir)/%/err
	rm -f $(stageDir)/$*.dir/diffs ;\
	for FILE in out err $($*.otherOutputs) ; do \
		echo "$(DIFF) $(stageDir)/$*.dir/$$FILE $(expDir)/$*/$$FILE" ;\
		$(DIFF) $(stageDir)/$*.dir/$$FILE $(expDir)/$*/$$FILE |& tee -a $(stageDir)/$*.dir/diffs ;\
	done ;\
	if [ -s $(stageDir)/$*.dir/diffs ] ; then \
		echo "Test $* failed due to diffs." ;\
		exit 1 ;\
	fi

# update expected files from current output files
# expected files always include out and err
# to avoid triggering dependencies, copy files over only if they are different or absent
%.expected: %.run
	mkdir -p $(expDir)/$*/ ;\
	for FILE in out err $($*.otherOutputs) ; do \
		$(DIFF) -qr $(stageDir)/$*.dir/$$FILE $(expDir)/$*/$$FILE \
		|| (echo "copying $(stageDir)/$*.dir/$$FILE to $(expDir)/$*/" ; cp $(stageDir)/$*.dir/$$FILE $(expDir)/$*/ ) ;\
	done

# copy outputs from expected files, to pretend that the module succeeded
%.pretend:	$(stageDir)/%.dir $(expDir)/%/out $(expDir)/%/err
	for FILE in out err $($*.otherOutputs) ; do \
		echo "copying $(expDir)/$*/$$FILE to $(stageDir)/$*.dir/" ;\
		cp $(expDir)/$*/$$FILE $(stageDir)/$*.dir/ ;\
	done

%.clean:
	-rm -rf $(stageDir)/$*.dir
	@echo "finished cleaning $(stageDir)/$*.dir"
