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

.PHONY:	default all stub help valid expected pretend run startTimer clean spotless %.valid %.expected %.run %.clean %.deps %.depsWrapper %.depsStart %.depsEnd %.stub

TOP_MK_DIR := $(patsubst %/,%,$(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
MIPS_HELP := $(shell lscpu | grep -i bogomips)
MIPS := $(word $(words $(MIPS_HELP)),$(MIPS_HELP))
ARCH ?= $(subst /,_,$(shell uname -o)_$(MIPS))
# can't use the architecture as directory name, cuz that produces different expected files on different platforms
# stageDir := $(ARCH)
stageDir := runs
dataDir := dataSets
expDir := expected
testCodeDir := $(TOP_MK_DIR)/testCode

allTests := $(patsubst %/,%,$(shell ls $(expDir)))
# allTests := 2

# the ?= allows the user to override the executable from the command line, as in
# make EXEC=myProgram all
# alas, $(realpath ...) doesn't seem to work here on make 3.81 for some reason
EXEC ?= ../vowpalwabbit/vw
TIME ?= env time -f "%U"
TIMING ?= n
ACC_DIFF_TOLERANCE ?= 0.001
TIME_DIFF_TOLERANCE ?= 0.05
DIFF ?= $(testCodeDir)/floatingTolerance.pl
GREP ?= grep
DEFAULT_STDOUT_COMPARATOR_REGEXP ?= "."
DEFAULT_STDERR_COMPARATOR_REGEXP ?= "average loss"

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
	echo $(ARCH)
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
run:	prepData startTimer $(runTargets)
	if [ "$(TIMING)" = "y" ] ; then \
		startTime=`cat startTime` ;\
		endTime=`date +%s` ;\
		elapsedTime=$$(($$endTime - $$startTime)) ;\
		echo "Total test time = $$elapsedTime" ;\
		rm startTime ;\
	fi
	@echo "'make run' finished"

startTimer:
	if [ "$(TIMING)" = "y" ] ; then \
		date +%s > startTime ;\
	fi

expectedTargets := $(addsuffix .expected,$(allTests))
expected:	$(expectedTargets)
	@echo "'make expected' finished"

pretendTargets := $(addsuffix .pretend,$(allTests))
pretend:	$(pretendTargets)
	@echo "'make pretend' finished"

clean:
	-rm -r $(stageDir)
	@echo "'make clean' finished"

%.dir/$(ARCH) %.dir:
	@mkdir -p $@

%.valid:	%.run %.compare ;

%.run:	 $$(%.inData) %.startNotice $(stageDir)/%.dir/done %.endNotice ;

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

# deps currently broken
# $(stageDir)/%.dir/done:	$(stageDir)/%.dir $$(%.deps) $$(%.inData)
$(stageDir)/%.dir/done:	$(stageDir)/%.dir $(stageDir)/%.dir/$(ARCH)
	exec=$(EXEC) ;\
	if [ -n "$($*.exec)" ] ; then exec="$($*.exec)" ; fi ;\
	roFile=$(stageDir)/$*.dir/raw_out ;\
	reFile=$(stageDir)/$*.dir/raw_err ;\
	oFile=$(stageDir)/$*.dir/out ;\
	eFile=$(stageDir)/$*.dir/err ;\
	if [ "$(TIMING)" = "y" ] ; then (\
		timeFile=$(stageDir)/$*.dir/$(ARCH)/rc ;\
		echo "$(TIME) -o $$timeFile $$exec $($*.params) $(EP) > $$roFile 2> $$reFile" ;\
		$(TIME) -o $$timeFile $$exec $($*.params) $(EP) > $$roFile 2> $$reFile ;\
		uTime=`cat $$timeFile` ;\
		echo "timing of test $* = $$uTime" \
	) ;\
	else (\
		echo "$$exec $($*.params) $(EP) > $$roFile 2> $$reFile" ;\
		$$exec $($*.params) $(EP) > $$roFile 2> $$reFile \
	) ; \
	fi ;\
	out_compar=$(DEFAULT_STDOUT_COMPARATOR_REGEXP) ;\
	if [ -n "$($*.STDOUT_COMPARATOR_REGEXP)" ] ; then out_compar="$($*.STDOUT_COMPARATOR_REGEXP)" ; fi ;\
	echo "$(GREP) \"$$out_compar\" $$roFile | tee $$oFile" ;\
	$(GREP) "$$out_compar" $$roFile | tee $$oFile ;\
	err_compar=$(DEFAULT_STDERR_COMPARATOR_REGEXP) ;\
	if [ -n "$($*.STDERR_COMPARATOR_REGEXP)" ] ; then err_compar="$($*.STDERR_COMPARATOR_REGEXP)" ; fi ;\
	echo "$(GREP) \"$$err_compar\" $$reFile | tee $$eFile" ;\
	$(GREP) "$$err_compar" $$reFile | tee $$eFile ;\
	touch $@

# first diff all the results, then set exit status if non-empty
%.compare:	$(stageDir)/%.dir $(expDir)/%/out $(expDir)/%/err
	rm -f $(stageDir)/$*.dir/diffs ;\
	for FILE in out err $($*.otherOutputs) ; do \
		echo "$(DIFF) $(ACC_DIFF_TOLERANCE) $(stageDir)/$*.dir/$$FILE $(expDir)/$*/$$FILE" ;\
		$(DIFF) $(ACC_DIFF_TOLERANCE) $(stageDir)/$*.dir/$$FILE $(expDir)/$*/$$FILE |& tee -a $(stageDir)/$*.dir/diffs ;\
	done ;\
	if [ -s $(stageDir)/$*.dir/diffs ] ; then \
		echo "Test $* failed due to accuracy diffs." ;\
		exit 1 ;\
	fi ;\
	if [ "$(TIMING)" = "y" ] ; then \
		echo "$(DIFF) $(TIME_DIFF_TOLERANCE) $(stageDir)/$*.dir/$(ARCH)/rc $(expDir)/$*/$(ARCH)/rc" ;\
		$(DIFF) $(TIME_DIFF_TOLERANCE) $(stageDir)/$*.dir/$(ARCH)/rc $(expDir)/$*/$(ARCH)/rc |& tee -a $(stageDir)/$*.dir/diffs ;\
		if [ -s $(stageDir)/$*.dir/diffs ] ; then \
			echo "Test $* failed due to speed diffs." ;\
			exit 1 ;\
		fi ;\
	fi

# update expected files from current output files
# expected files always include out and err
%.expected:
	mkdir -p $(expDir)/$*/ ;\
	for FILE in out err $($*.otherOutputs) ; do \
		(echo "copying $(stageDir)/$*.dir/$$FILE to $(expDir)/$*/" ; cp $(stageDir)/$*.dir/$$FILE $(expDir)/$*/ ) ;\
	done ;\
	if [ "$(TIMING)" = "y" ] ; then \
		mkdir -p $(expDir)/$*/$(ARCH) ;\
		(echo "copying $(stageDir)/$*.dir/$(ARCH)/rc to $(expDir)/$*/$(ARCH)/rc" ; cp $(stageDir)/$*.dir/$(ARCH)/rc $(expDir)/$*/$(ARCH)/rc ) ;\
	fi

# copy outputs from expected files, to pretend that the module succeeded
%.pretend:	$(stageDir)/%.dir $(expDir)/%/out $(expDir)/%/err
	for FILE in out err $($*.otherOutputs) ; do \
		echo "copying $(expDir)/$*/$$FILE to $(stageDir)/$*.dir/" ;\
		cp $(expDir)/$*/$$FILE $(stageDir)/$*.dir/ ;\
	done ;\
	if [ "$(TIMING)" = "y" ] ; then \
		mkdir -p $(stageDir)/$*.dir/$(ARCH) ;\
		echo "copying $(expDir)/$*/$(ARCH)/rc $(stageDir)/$*.dir/$(ARCH)/rc" ;\
		cp $(expDir)/$*/$(ARCH)/rc $(stageDir)/$*.dir/$(ARCH)/rc ;\
	fi

%.clean:
	-rm -rf $(stageDir)/$*.dir
	@echo "finished cleaning $(stageDir)/$*.dir"
