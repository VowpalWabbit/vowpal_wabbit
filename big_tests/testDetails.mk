allTests := 1.test 2.test

# executable should be listed relative to top level Makefile
# by default, STDOUT will go to file out and STDERR will go to file err
# other created files that should be compared to "expected" files should be captured by the *.otherOutputs variable
# list inter-test dependencies, like below:
# 31.deps:	30.run 29.run

# by default there are no deps
%.deps:
	echo

107.exec = $(TOP_MK_DIR)/../vowpalwabbit/vw
107.inData = ftrl-proximal-train.prep
107.params = -k -t $(107.inData) -f 0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off
107.otherOutputs = ""

