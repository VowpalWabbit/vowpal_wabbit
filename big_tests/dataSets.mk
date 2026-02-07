# Makefile for obtaining and preparing data sets

allData := mnist covtype URLRep ER movielens

################ begin generic stuff #########

VPATH+=$(testCodeDir)
WGET ?= wget -nv -N --no-use-server-timestamps --no-check-certificate

.PHONY:	getData prepData archive eraseData

# archive file name can be specified on the command line as ARF=<filename>
archive:
	cd $(dataDir)
	find . -name '*prep' -print0 | tar -cjhv --null -f $(ARF) -T -

getData:	$(dataDir)
	cd $(dataDir)
	$(WGET) $(URL)
	fName=`basename $(URL)` ; tar xjvmf $$fName

# If a URL is specified, then simply download all the data
# pre-prepped.  Checked-in check-sums should be used to guard against
# something missing or corrupted in the archive.
allDataTargets := $(addsuffix .prep,$(allData))
ifdef URL
prepData:	getData
else
prepData:	$(allDataTargets)
endif
	@echo "finished preparing all data"

%.prep:	$(dataDir)/%.dir/prep ;

# allDataDirs := $(addprefix $(dataDir)/,$(addsuffix .dir,$(allData)))
eraseData:
	-rm -r $(dataDir)
	@echo "finished erasing all data"

################ end generic stuff #########

#OCR
OCR.prep:	$(dataDir)/OCR.dir/train.prep $(dataDir)/OCR.dir/test.prep ;
$(dataDir)/OCR.dir/train.prep:	$(dataDir)/OCR.dir/test.prep ;
$(dataDir)/OCR.dir/test.prep:	$(dataDir)/OCR.dir/letter.data.gz $(dataDir)/OCR.dir/letter.names
	dir=$(dir $@) ;\
	cd $$dir ;\
	$(testCodeDir)/ocr2vw.py letter.data.gz letter.names train.prep test.prep

$(dataDir)/OCR.dir/letter.data.gz:
	dir=$(dir $@) ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	$(WGET) http://ai.stanford.edu/~btaskar/ocr/letter.data.gz

$(dataDir)/OCR.dir/letter.names:
	dir=$(dir $@) ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	$(WGET) http://ai.stanford.edu/~btaskar/ocr/letter.names


#movielens
movielens.prep:	$(dataDir)/movielens.dir/train.prep ;
$(dataDir)/movielens.dir/train.prep:	$(dataDir)/movielens.dir/test.prep ;
	cd $(dataDir)/movielens.dir/ ;\
	perl -ne 'BEGIN { srand 8675309; };             \
		1; print join "\t", rand (), $$_;'          \
		pre.train.vw | sort -k1 |     \
		cut -f2- > train.prep

$(dataDir)/movielens.dir/test.prep:	$(dataDir)/movielens.dir/ml-1m.zip
	cd $(dataDir)/movielens.dir/ ;\
	unzip -ou ml-1m.zip ;\
	$(testCodeDir)/movielensRatings2vw.pl pre.train.vw test.prep ml-1m/ratings.dat

$(dataDir)/movielens.dir/ml-1m.zip:
	dir=$(dir $@) ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	$(WGET) http://files.grouplens.org/datasets/movielens/ml-1m.zip


#ER
ER.prep:	$(dataDir)/ER.dir/train.prep $(dataDir)/ER.dir/test.prep ;
$(dataDir)/ER.dir/train.prep:	$(dataDir)/ER.dir/ER_train.vw
	cd $(dataDir)/ER.dir/ ;\
	ln -sf ER_train.vw train.prep
$(dataDir)/ER.dir/test.prep:	$(dataDir)/ER.dir/ER_test.vw
	cd $(dataDir)/ER.dir/ ;\
	ln -sf ER_test.vw test.prep

$(dataDir)/ER.dir/ER_train.vw:	$(dataDir)/ER.dir/ER_test.vw ;
	touch $@
$(dataDir)/ER.dir/ER_test.vw:	$(dataDir)/ER.dir/er.zip
	cd $(dataDir)/ER.dir/ ;\
	unzip -ou er.zip ;\
	touch ER_test.vw

$(dataDir)/ER.dir/er.zip:
	dir=$(dir $@) ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	$(WGET) http://web.engr.illinois.edu/~kchang10/data/er.zip

# URLRep
$(dataDir)/URLRep.dir/prep:	$(dataDir)/URLRep.dir/url_svmlight.tar.gz URLRep.munge.sh
	export testCodeDir=$(testCodeDir) ;\
	cd $(dataDir)/URLRep.dir/ ;\
	$(testCodeDir)/URLRep.munge.sh url_svmlight.tar.gz > prep

$(dataDir)/URLRep.dir/url_svmlight.tar.gz:
	dir=$(dir $@) ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	$(WGET) https://archive.ics.uci.edu/ml/machine-learning-databases/url/url_svmlight.tar.gz


# COVERTYPE
$(dataDir)/covtype.dir/prep:	$(dataDir)/covtype.dir/covtype.data.gz covtype.munge.sh
	export testCodeDir=$(testCodeDir) ;\
	cd $(dataDir)/covtype.dir/ ;\
	$(testCodeDir)/covtype.munge.sh covtype.data.gz > prep

$(dataDir)/covtype.dir/covtype.data.gz:
	dir=$(dir $@) ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	$(WGET) https://archive.ics.uci.edu/ml/machine-learning-databases/covtype/covtype.data.gz


## MNIST
# override implicit %.prep rule
mnist.prep:	$(dataDir)/mnist.dir/train.prep $(dataDir)/mnist.dir/test.prep ;

$(dataDir)/mnist.dir/train.prep:	mnist.extractfeatures mnist.extract-labels.pl shuffle.pl $(dataDir)/mnist.dir/train-labels-idx1-ubyte.gz $(dataDir)/mnist.dir/train-images-idx3-ubyte.gz mnist.munge.sh
	export testCodeDir=$(testCodeDir) ;\
	cd $(dataDir)/mnist.dir/ ;\
	$(testCodeDir)/mnist.munge.sh train-labels-idx1-ubyte.gz train-images-idx3-ubyte.gz \
	| $(testCodeDir)/shuffle.pl > train.prep

$(dataDir)/mnist.dir/test.prep:	mnist.munge.sh mnist.extractfeatures mnist.extract-labels.pl $(dataDir)/mnist.dir/t10k-labels-idx1-ubyte.gz $(dataDir)/mnist.dir/t10k-images-idx3-ubyte.gz
	export testCodeDir=$(testCodeDir) ;\
	cd $(dataDir)/mnist.dir/ ;\
	$(testCodeDir)/mnist.munge.sh t10k-labels-idx1-ubyte.gz t10k-images-idx3-ubyte.gz > test.prep

mnist.extractfeatures:	mnist.extractfeatures.cpp
	cd $(testCodeDir)/ ;\
	g++ -O3 -Wall $^ -o $@

$(dataDir)/mnist.dir/%.gz:
	dir=$(dir $@) ;\
	mkdir -p $$dir ;\
	cd $$dir ;\
	fileName=`basename $@` ;\
	$(WGET) https://storage.googleapis.com/cvdf-datasets/mnist/$$fileName

