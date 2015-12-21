
# test groups

# this test group is bogus, for illustration only; adjust if you know what you're doing
regression_group:	1a.valid 1b.valid 3.valid ;

# individual tests

# MNIST training
1a.inData := mnist.dir/mnist-train.prep
1a.params := --oaa 10 -d $(dataDir)/$(1a.inData) -f mnist.model -b 24 --adaptive --invariant --holdout_off -l 0.1 --nn 40 --passes 24 -k --compressed --cache_file mnist.cache

# MNIST prediction
1b.inData := mnist.dir/mnist-test.prep
1b.params := -t -d $(dataDir)/$(1b.inData) -i $(stageDir)/1a.dir/mnist.model
1b.deps := 1a.valid

# COVERTYPE
2.inData := covtype.dir/covtype.prep
2.params := --oaa 7 -d $(dataDir)/$(2.inData)

# URL Reputation
3.inData := URLRep.dir/URLRep.prep
3.params := -d $(dataDir)/$(3.inData)
