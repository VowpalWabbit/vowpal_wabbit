# Makefile for obtaining and preparing data sets

allData := mnist covtype URLRep

################ begin generic stuff #########

mungeCodeDir := $(TOP_MK_DIR)/dataPrepCode

.PHONY:	getData eraseData %.prep

allDataTargets := $(addsuffix .prep,$(allData))
getData:	$(allDataTargets)
	@echo "finished preparing all data"

%.prep:	$(dataDir)/%.dir $(dataDir)/%.dir/prep ;

# allDataDirs := $(addprefix $(dataDir)/,$(addsuffix .dir,$(allData)))
eraseData:
	-rm -r $(dataDir)
	@echo "finished erasing all data"

################ end generic stuff #########

# URLRep
$(dataDir)/URLRep.dir/prep:	$(mungeCodeDir)/URLRep.munge.sh URLRep.raw
	export mungeCodeDir=$(mungeCodeDir) ;\
	cd $(dataDir)/URLRep.dir/ ;\
	$(mungeCodeDir)/URLRep.munge.sh url_svmlight.tar.gz > prep

URLRep.raw: $(dataDir)/URLRep.dir $(dataDir)/URLRep.dir/url_svmlight.tar.gz

$(dataDir)/URLRep.dir/url_svmlight.tar.gz:
	dir=`dirname $@` ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	wget https://archive.ics.uci.edu/ml/machine-learning-databases/url/url_svmlight.tar.gz


# COVERTYPE
$(dataDir)/covtype.dir/prep:	$(mungeCodeDir)/covtype.munge.sh covtype.raw
	export mungeCodeDir=$(mungeCodeDir) ;\
	cd $(dataDir)/covtype.dir/ ;\
	$(mungeCodeDir)/covtype.munge.sh covtype.data.gz > prep

covtype.raw: $(dataDir)/covtype.dir $(dataDir)/covtype.dir/covtype.data.gz ;

$(dataDir)/covtype.dir/covtype.data.gz:
	dir=`dirname $@` ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	wget https://archive.ics.uci.edu/ml/machine-learning-databases/covtype/covtype.data.gz


## MNIST
# override implicit %.prep rule
mnist.prep:	$(dataDir)/mnist.dir $(dataDir)/mnist.dir/train.prep $(dataDir)/mnist.dir/test.prep ;

$(dataDir)/mnist.dir/train.prep:	$(mungeCodeDir)/mnist.munge.sh $(mungeCodeDir)/mnist.extractfeatures $(mungeCodeDir)/mnist.extract-labels.pl mnist-train.raw
	export mungeCodeDir=$(mungeCodeDir) ;\
	cd $(dataDir)/mnist.dir/ ;\
	$(mungeCodeDir)/mnist.munge.sh train-labels-idx1-ubyte.gz train-images-idx3-ubyte.gz \
	| $(mungeCodeDir)/shuffle.pl > train.prep

$(mungeCodeDir)/mnist.extractfeatures:	$(mungeCodeDir)/mnist.extractfeatures.cpp
	cd $(mungeCodeDir)/ ;\
	g++ -O3 -Wall $^ -o $@

mnist-train.raw: $(dataDir)/mnist.dir $(dataDir)/mnist.dir/train-labels-idx1-ubyte.gz $(dataDir)/mnist.dir/train-images-idx3-ubyte.gz ;

$(dataDir)/mnist.dir/%.gz:
	dir=`dirname $@` ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	fileName=`basename $@` ;\
	wget http://yann.lecun.com/exdb/mnist/$$fileName

$(dataDir)/mnist.dir/test.prep:	$(mungeCodeDir)/mnist.munge.sh $(mungeCodeDir)/mnist.extractfeatures $(mungeCodeDir)/mnist.extract-labels.pl mnist-test.raw
	export mungeCodeDir=$(mungeCodeDir) ;\
	cd $(dataDir)/mnist.dir/ ;\
	$(mungeCodeDir)/mnist.munge.sh t10k-labels-idx1-ubyte.gz t10k-images-idx3-ubyte.gz > test.prep

mnist-test.raw:	$(dataDir)/mnist.dir $(dataDir)/mnist.dir/t10k-labels-idx1-ubyte.gz $(dataDir)/mnist.dir/t10k-images-idx3-ubyte.gz ;
