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

ifdef TIMING
TIMING_SUFFIX = WithTiming
MIPS_HELP := $(shell lscpu | grep -i bogomips)
MIPS := $(word $(words $(MIPS_HELP)),$(MIPS_HELP))
ARCH ?= $(subst /,_,$(shell uname -o)_$(MIPS))
# can't use the architecture as directory name, cuz that produces different expected files on different platforms
# stageDir := $(ARCH)
TIME ?= env time -f "%U"
TIME_DIFF_TOLERANCE_REL ?= 0.05
TIME_DIFF_TOLERANCE_ABS ?= 1.0
endif

stageDir ?= runs
dataDir ?= dataSets
expDir ?= expected
testCodeDir := $(TOP_MK_DIR)/testCode

allTests := $(patsubst %/,%,$(shell ls $(expDir)))
# allTests := 2

# the ?= allows the user to override the executable from the command line, as in
# make EXEC=myProgram all
# alas, $(realpath ...) doesn't seem to work here on make 3.81 for some reason
EXEC ?= ../vowpalwabbit/vw
ACC_DIFF_TOLERANCE_REL ?= 0.001
ACC_DIFF_TOLERANCE_ABS ?= 0.001
DIFF ?= $(testCodeDir)/floatingTolerance.pl
GREP ?= grep
DEFAULT_STDOUT_COMPARATOR_REGEXP ?= "."
DEFAULT_STDERR_COMPARATOR_REGEXP ?= "average loss"
ARF ?= vwBigtests.tz2
# you can uncomment the next line to get the data from a cache by default, but we don't recommend it
URL ?= https://vowpalwabbitdata.blob.core.windows.net/bigtests/vwBigtests.tz2


all:	valid ;

.DEFAULT:
	@echo "No rules found for $@ in $(MAKEFILE_LIST)"

# details of individual tests and test groups come from here
include dataSets.mk
include testDetails.mk
include testGroups.mk

# for debugging
%.stub:
	echo $($*.deps)
	echo $(stageDir)/$*.dir/done
	echo $(addsuffix .valid,$($*.deps))

# for debugging
stub:	
	echo $(TIMING)
	echo $(MAKE_VERSION)
	echo $(ARCH)
	echo $(MAKEFLAGS)
	echo $(EXEC)
	echo $(TOP_MK_DIR)
	echo $(VPATH)

help:
	cat $(TOP_MK_DIR)/README.txt

validTargets := $(addsuffix .valid,$(allTests))
valid:	$(validTargets) timingSummary
	@echo "test suite finished"

# use this target to run without doing any diffs
runTargets := $(addsuffix .run,$(allTests))
run:	prepData $(runTargets) timingSummary
	@echo "'make run' finished"

timingSummary:
	if [ "$(TIMING)" = "y" ] ; then \
		totalTime=0 ;\
		for dir in $(allTests) ; do \
			timeFile=$(stageDir)/$$dir.dir/$(ARCH)/rc ;\
			currTime=`cat $$timeFile` ;\
			totalTime=`echo "$$totalTime + $$currTime" | bc -l` ;\
		done ;\
		echo "Total test time = $$totalTime" ;\
	fi

mcompareTargets := $(addsuffix .compare,$(allTests))
compare:	$(compareTargets)
	@echo "'make compare' finished"

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

# dependency tracking doesn't work in make < 4 due to a bug in how SECONDEXPANSION is done
ifeq ($(findstring 4.,$(MAKE_VERSION)),4.)
%.run:	 %.depsWrapper $(stageDir)/%.dir/done$(TIMING_SUFFIX) ;
else
%.run:	 $(stageDir)/%.dir/done$(TIMING_SUFFIX) ;
endif

$(dataDir) $(stageDir):
	@mkdir -p $@

%.depsWrapper:	%.depsStart $$(%.deps) %.depsEnd ;

%.depsStart:
	@if [ -n "$($*.deps)" ] ; then \
		echo "test $* has dependencies: $($*.deps)" ;\
	fi

%.depsEnd:
	@if [ -n "$($*.deps)" ] ; then \
		echo "dependencies of test $* satisfied" ;\
	fi

# Don't do like on the following line, because making the dir a dependency of the file that's in it will result in the file always getting remade, cuz the dir will always have a newer timestamp.
# $(stageDir)/%.dir/done:	$(stageDir)/%.dir
$(stageDir)/%.dir/doneWithTiming:	$(EXEC) $$(%.inData)
	@echo "Starting to run test $*"
	dir=`dirname $@` ;\
	mkdir -p $$dir ;\
	exec=$(EXEC) ;\
	if [ -n "$($*.exec)" ] ; then exec="$($*.exec)" ; fi ;\
	roFile=$$dir/raw_out ;\
	reFile=$$dir/raw_err ;\
	oFile=$$dir/out ;\
	eFile=$$dir/err ;\
	mkdir -p $$dir/$(ARCH) ;\
	timeFile=$$dir/$(ARCH)/rc ;\
	echo "$(TIME) -o $$timeFile $$exec $($*.params) $(EP) > $$roFile 2> $$reFile" ;\
	$(TIME) -o $$timeFile $$exec $($*.params) $(EP) > $$roFile 2> $$reFile ;\
	uTime=`cat $$timeFile` ;\
	echo "run time of test $* = $$uTime" ;\
	out_compar=$(DEFAULT_STDOUT_COMPARATOR_REGEXP) ;\
	if [ -n "$($*.STDOUT_COMPARATOR_REGEXP)" ] ; then out_compar="$($*.STDOUT_COMPARATOR_REGEXP)" ; fi ;\
	echo "$(GREP) \"$$out_compar\" $$roFile | tee $$oFile" ;\
	$(GREP) "$$out_compar" $$roFile | tee $$oFile ;\
	err_compar=$(DEFAULT_STDERR_COMPARATOR_REGEXP) ;\
	if [ -n "$($*.STDERR_COMPARATOR_REGEXP)" ] ; then err_compar="$($*.STDERR_COMPARATOR_REGEXP)" ; fi ;\
	echo "$(GREP) \"$$err_compar\" $$reFile | tee $$eFile" ;\
	$(GREP) "$$err_compar" $$reFile | tee $$eFile ;\
	touch $$dir/done
	touch $@
	@echo "Finished running test $*"

$(stageDir)/%.dir/done:	$(EXEC) $$(%.inData)
	@echo "Starting to run test $*"
	dir=`dirname $@` ;\
	mkdir -p $$dir ;\
	exec=$(EXEC) ;\
	if [ -n "$($*.exec)" ] ; then exec="$($*.exec)" ; fi ;\
	roFile=$$dir/raw_out ;\
	reFile=$$dir/raw_err ;\
	oFile=$$dir/out ;\
	eFile=$$dir/err ;\
	echo "$$exec $($*.params) $(EP) > $$roFile 2> $$reFile" ;\
	$$exec $($*.params) $(EP) > $$roFile 2> $$reFile ;\
	out_compar=$(DEFAULT_STDOUT_COMPARATOR_REGEXP) ;\
	if [ -n "$($*.STDOUT_COMPARATOR_REGEXP)" ] ; then out_compar="$($*.STDOUT_COMPARATOR_REGEXP)" ; fi ;\
	echo "$(GREP) \"$$out_compar\" $$roFile | tee $$oFile" ;\
	$(GREP) "$$out_compar" $$roFile | tee $$oFile ;\
	err_compar=$(DEFAULT_STDERR_COMPARATOR_REGEXP) ;\
	if [ -n "$($*.STDERR_COMPARATOR_REGEXP)" ] ; then err_compar="$($*.STDERR_COMPARATOR_REGEXP)" ; fi ;\
	echo "$(GREP) \"$$err_compar\" $$reFile | tee $$eFile" ;\
	$(GREP) "$$err_compar" $$reFile | tee $$eFile ;\
	touch $@
	@echo "Finished running test $*"

# first diff all the results, then set exit status if non-empty
%.compare:	$(DIFF) $(expDir)/%/out $(expDir)/%/err
	dir=$(stageDir)/$*.dir ;\
	rm -f $$dir/diffs ;\
	for FILE in out err $($*.otherOutputs) ; do \
		echo "$(DIFF) $(ACC_DIFF_TOLERANCE_REL) $(ACC_DIFF_TOLERANCE_ABS) $$dir/$$FILE $(expDir)/$*/$$FILE" ;\
		$(DIFF) $(ACC_DIFF_TOLERANCE_REL) $(ACC_DIFF_TOLERANCE_ABS) $$dir/$$FILE $(expDir)/$*/$$FILE |& tee -a $$dir/diffs ;\
	done ;\
	if [ -s $$dir/diffs ] ; then \
		echo "Test $* failed due to accuracy diffs." ;\
		exit 1 ;\
	fi ;\
	if [ "$(TIMING)" = "y" ] ; then \
		timeFile=$$dir/$(ARCH)/rc ;\
		uTime=`cat $$timeFile` ;\
		echo "run time of test $* = $$uTime" ;\
		if [ -s $(expDir)/$*/$(ARCH)/rc ] ; then (\
			echo "$(DIFF) $(TIME_DIFF_TOLERANCE_REL) $(TIME_DIFF_TOLERANCE_ABS) $$timeFile $(expDir)/$*/$(ARCH)/rc" ;\
			$(DIFF) $(TIME_DIFF_TOLERANCE_REL) $(TIME_DIFF_TOLERANCE_ABS) $$timeFile $(expDir)/$*/$(ARCH)/rc |& tee -a $$dir/diffs ;\
			if [ -s $$dir/diffs ] ; then \
				echo "Test $* failed due to speed diffs." ;\
				exit 1 ;\
			fi ;\
		) ;\
		else (\
			echo "WARNING: Missing expected timing file.  You might want to create it with 'make TIMING=y expected'.  Timing validation skipped." \
		) ; \
		fi ;\
	fi

# update expected files from current output files
# expected files always include out and err
%.expected:
	dir=$(stageDir)/$*.dir ;\
	mkdir -p $(expDir)/$*/ ;\
	for FILE in out err $($*.otherOutputs) ; do \
		(echo "copying $$dir/$$FILE to $(expDir)/$*/" ; cp $$dir/$$FILE $(expDir)/$*/ ) ;\
	done ;\
	if [ "$(TIMING)" = "y" ] ; then \
		mkdir -p $(expDir)/$*/$(ARCH) ;\
		(echo "copying $$dir/$(ARCH)/rc to $(expDir)/$*/$(ARCH)/rc" ; cp $$dir/$(ARCH)/rc $(expDir)/$*/$(ARCH)/rc ) ;\
	fi

# copy outputs from expected files, to pretend that the module succeeded
%.pretend:	$(stageDir)/%.dir $(expDir)/%/out $(expDir)/%/err
	dir=$(stageDir)/$*.dir ;\
	for FILE in out err $($*.otherOutputs) ; do \
		echo "copying $(expDir)/$*/$$FILE to $$dir/" ;\
		cp $(expDir)/$*/$$FILE $$dir/ ;\
	done ;\
	touch $$dir/done ;\
	if [ "$(TIMING)" = "y" ] ; then \
		mkdir -p $$dir/$(ARCH) ;\
		echo "copying $(expDir)/$*/$(ARCH)/rc $$dir/$(ARCH)/rc" ;\
		cp $(expDir)/$*/$(ARCH)/rc $$dir/$(ARCH)/rc ;\
	fi

%.clean:
	-rm -rf $(stageDir)/$*.dir
	@echo "finished cleaning $(stageDir)/$*.dir"
