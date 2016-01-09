
# test groups

# this test group is bogus, for illustration only; adjust if you know what you're doing
regression_group:	1a.valid 1b.valid 3.valid ;


# individual tests

# MNIST training
1a.inData := $(dataDir)/mnist.dir/train.prep
1a.params := --oaa 10 -d $(1a.inData) -f $(stageDir)/1a.dir/mnist.model -b 24 --adaptive --invariant --holdout_off -l 0.1 --nn 40 --passes 24 -k --compressed --cache_file $(stageDir)/1a.dir/mnist.cache

# MNIST prediction
1b.inData := $(dataDir)/mnist.dir/test.prep
1b.params := -t -d $(1b.inData) -i $(stageDir)/1a.dir/mnist.model
# test dependencies not working yet
# 1b.deps = 1a.valid

# COVERTYPE
2.inData := $(dataDir)/covtype.dir/prep
2.params := --oaa 7 -d $(2.inData)

# URL Reputation
3.inData := $(dataDir)/URLRep.dir/prep
3.params := -d $(3.inData)

# Entity Relation training
4a.inData := $(dataDir)/ER.dir/train.prep
4a.params := -b 24 -d $(4a.inData) --search 10 --passes 10 --search_task entity_relation --constraints --search_alpha 1e-8 -f $(stageDir)/4a.dir/er.model --cache_file $(stageDir)/4a.dir/er.cache

# Entity Relation prediction
4b.inData := $(dataDir)/ER.dir/test.prep
4b.params := -t -d $(4b.inData) -i $(stageDir)/4a.dir/er.model
