
# scripts
# SCRIPT_DIR = $(BDTL_BASE)/scripts

GDIFF = diff
XARGS = xargs

# validate
%.valid:	%.run %.check

%.run:	%.deps $(stageDir)/%.dir/out $(stageDir)/%.dir/err
	@echo "Finished $@"

#.ONESHELL: doesn't seem to work
$(stageDir)/%.dir/out $(stageDir)/%.dir/err:	$(%.exec) $(stageDir)/%.dir $(dataDir)/$($*.inData)
	cd $(stageDir)/$*.dir ;\
	$($*.exec) $($*.params) > out 2> err

$(stageDir)/%.dir:
	mkdir -p $@

# first diff all the diffTargets, then exit if any of them don't match
%.check:
	for FILE in out err $*.otherOutputs ; do
		echo "($(GDIFF) -qb $$FILE $(expDir)/$$FILE.expected)"
		($(GDIFF) -qb $$FILE $$base)
	done

# update expected files from current target files
# $*.out and $*.err are always added as expected files
# copy files over only if they are actually different, to avoid triggering dependencies
%.expected: %.run
	for FILE in $*.out.expected $*.err.expected $*.expectedFiles ; do
		base=`basename $$FILE .expected`
		($(GDIFF) -qb $$FILE $$base)
		|| (echo "copying $$base to $$FILE" ; mv $$base $$FILE)
		|| exit
	done

# create targets from expected files, to pretend that the module succeeded
%.pretend:	%.expectedFiles
	for FILE in $*.out.expected $*.err.expected $*.expectedFiles ; do
		base=`basename $$FILE .expected`
		echo "copying $$FILE to $$base"
		cp $$FILE $$base || exit
	done

%.clean:
	-rm -rf $(stageDir)/$*
