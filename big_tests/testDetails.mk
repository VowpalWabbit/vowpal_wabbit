# The only variables that are required for every test are *.inData and *.params .  Everything else is optional.  See examples below.

# If you're using an unusual executable (e.g. library_example), its path should be listed relative to the dir of the top level Makefile.

# By default, STDOUT will go to the file out and STDERR will go to the file err. Other created files that should be compared to "expected" files should be captured by the *.otherOutputs variable. The other expected files should be copied to the test's subdirectory under expected/ .

# List inter-test dependencies like this:
# 31.deps:	30.run 29.test
# Dependencies can be on any set of valid targets, including merely running other tests (*.run targets) or those tests actually passing (*.test targets).

107.exec = $(TOP_MK_DIR)/../vowpalwabbit/vw
107.inData = ftrl-proximal/ftrl-proximal-train.prep
107.params = -k -d $(dataDir)/$(107.inData) -f 0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off

2.inData = ftrl-proximal/ftrl-proximal-train.prep
2.params = -k -d $(dataDir)/$(2.inData) -f 0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off
2.otherOutputs = 0001_ftrl.model
2.deps = 107.test
