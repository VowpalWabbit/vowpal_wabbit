
# test groups

# this test group is bogus, for illustration only; adjust if you know what you're doing
regression_group:	1a.valid 1b.valid 3.valid ;


# individual tests

# MNIST training
1a.inData := $(dataDir)/mnist.dir/train.prep
1a.params := --oaa 10 -d $(1a.inData) -f mnist.model -b 24 --adaptive --invariant --holdout_off -l 0.1 --nn 40 --passes 24 -k --compressed --cache_file mnist.cache

# MNIST prediction
1b.inData := $(dataDir)/mnist.dir/test.prep
1b.params := -t -d $(1b.inData) -i $(stageDir)/1a.dir/mnist.model
1b.deps := 1a.valid

# COVERTYPE
2.inData := $(dataDir)/covtype.dir/prep
2.params := --oaa 7 -d $(2.inData)

# URL Reputation
3.inData := $(dataDir)/URLRep.dir/prep
3.params := -d $(3.inData)
