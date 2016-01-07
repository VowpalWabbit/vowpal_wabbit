# Makefile for obtaining and preparing data sets

allData := mnist covtype URLRep

################ begin generic stuff #########

VPATH+=$(testCodeDir)

.PHONY:	prepData eraseData %.prep

allDataTargets := $(addsuffix .prep,$(allData))
prepData:	$(allDataTargets)
	@echo "finished preparing all data"

%.prep:	$(dataDir)/%.dir $(dataDir)/%.dir/prep ;

# allDataDirs := $(addprefix $(dataDir)/,$(addsuffix .dir,$(allData)))
eraseData:
	-rm -r $(dataDir)
	@echo "finished erasing all data"

################ end generic stuff #########

# URLRep
$(dataDir)/URLRep.dir/prep:	URLRep.raw URLRep.munge.sh
	export testCodeDir=$(testCodeDir) ;\
	cd $(dataDir)/URLRep.dir/ ;\
	$(testCodeDir)/URLRep.munge.sh url_svmlight.tar.gz > prep

URLRep.raw: $(dataDir)/URLRep.dir $(dataDir)/URLRep.dir/url_svmlight.tar.gz

$(dataDir)/URLRep.dir/url_svmlight.tar.gz:
	dir=`dirname $@` ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	wget https://archive.ics.uci.edu/ml/machine-learning-databases/url/url_svmlight.tar.gz


# COVERTYPE
$(dataDir)/covtype.dir/prep:	covtype.raw covtype.munge.sh
	export testCodeDir=$(testCodeDir) ;\
	cd $(dataDir)/covtype.dir/ ;\
	$(testCodeDir)/covtype.munge.sh covtype.data.gz > prep

covtype.raw: $(dataDir)/covtype.dir $(dataDir)/covtype.dir/covtype.data.gz ;

$(dataDir)/covtype.dir/covtype.data.gz:
	dir=`dirname $@` ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	wget https://archive.ics.uci.edu/ml/machine-learning-databases/covtype/covtype.data.gz


## MNIST
# override implicit %.prep rule
mnist.prep:	$(dataDir)/mnist.dir $(dataDir)/mnist.dir/train.prep $(dataDir)/mnist.dir/test.prep ;

$(dataDir)/mnist.dir/train.prep:	 mnist-train.raw mnist.munge.sh mnist.extractfeatures mnist.extract-labels.pl shuffle.pl
	export testCodeDir=$(testCodeDir) ;\
	cd $(dataDir)/mnist.dir/ ;\
	$(testCodeDir)/mnist.munge.sh train-labels-idx1-ubyte.gz train-images-idx3-ubyte.gz \
	| $(testCodeDir)/shuffle.pl > train.prep

mnist.extractfeatures:	mnist.extractfeatures.cpp
	cd $(testCodeDir)/ ;\
	g++ -O3 -Wall $^ -o $@

mnist-train.raw: $(dataDir)/mnist.dir $(dataDir)/mnist.dir/train-labels-idx1-ubyte.gz $(dataDir)/mnist.dir/train-images-idx3-ubyte.gz ;

$(dataDir)/mnist.dir/%.gz:
	dir=`dirname $@` ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	fileName=`basename $@` ;\
	wget http://yann.lecun.com/exdb/mnist/$$fileName

$(dataDir)/mnist.dir/test.prep:	mnist-test.raw mnist.munge.sh mnist.extractfeatures mnist.extract-labels.pl
	export testCodeDir=$(testCodeDir) ;\
	cd $(dataDir)/mnist.dir/ ;\
	$(testCodeDir)/mnist.munge.sh t10k-labels-idx1-ubyte.gz t10k-images-idx3-ubyte.gz > test.prep

mnist-test.raw:	$(dataDir)/mnist.dir $(dataDir)/mnist.dir/t10k-labels-idx1-ubyte.gz $(dataDir)/mnist.dir/t10k-images-idx3-ubyte.gz ;
