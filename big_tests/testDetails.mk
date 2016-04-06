
# MNIST training
1a.inData := $(dataDir)/mnist.dir/train.prep
1a.params := --oaa 10 -d $(1a.inData) -f $(stageDir)/1a.dir/mnist.model -b 24 --adaptive --invariant --holdout_off -l 0.1 --nn 40 --passes 24 -k --compressed --cache_file $(stageDir)/1a.dir/mnist.cache

# MNIST prediction
1b.inData := $(dataDir)/mnist.dir/test.prep
1b.params := -t -d $(1b.inData) -i $(stageDir)/1a.dir/mnist.model
# test dependencies not working yet
1b.deps := 1a.valid

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
4b.deps := 4a.valid

# MovieLens training
5a.inData := $(dataDir)/movielens.dir/train.prep
5a.params := --loss_function quantile -l 0.45 -b 24 --passes 100 -k --cache_file $(stageDir)/5a.dir/movielens.cache -d $(5a.inData) --holdout_off --lrq um14 --lrqdropout --adaptive --invariant -f $(stageDir)/5a.dir/movielens.model

# MovieLens prediction
5b.inData := $(dataDir)/movielens.dir/test.prep
5b.params := --loss_function quantile -t -i $(stageDir)/5a.dir/movielens.model -d $(5b.inData)
5b.deps := 5a.valid

# OCR training
6a.inData := $(dataDir)/OCR.dir/train.prep
6a.params := -d $(6a.inData) -f $(stageDir)/6a.dir/OCR.model --cache_file $(stageDir)/6a.dir/OCR.cache -k --oaa 26 --adaptive --invariant --holdout_off --loss_function logistic --passes 14

# OCR prediction
6b.inData := $(dataDir)/OCR.dir/test.prep
6b.params := -i $(stageDir)/6a.dir/OCR.model -d $(6b.inData) --testonly
6b.deps := 6a.valid
