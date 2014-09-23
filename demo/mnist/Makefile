.SECONDARY:
CXXFLAGS=-O3 -Wall

help:
	@cat README

RAW = mnist mnist8m
PNG = mnistpng mnist11png mnist8mpng mnist8m11png

raw : $(addsuffix .test.confusion, $(RAW))

png : $(addsuffix .test.confusion, $(PNG))

MNIST = mnist mnistpng mnist11png
8M = mnist8m mnist8mpng mnist8m11png

mnist : $(addsuffix .test.confusion, $(MNIST))

mnist8m : $(addsuffix .test.confusion, $(8M))

.PHONY : clean testclean %.test.confusion mnist mnist8m raw png

%.check:
	@test -x "$$(which $*)" || {				\
	  echo "ERROR: you need to install $*" 1>&2;		\
	  exit 1;						\
	}

testclean:
	rm -f $(wildcard *.test.predictions)

clean:
	rm -f $(wildcard mnist8mpng.*) $(wildcard mnist8m11png.*) $(wildcard mnist11png.*) $(wildcard mnistpng.*) $(wildcard mnist8m.*) $(wildcard mnist.*)

# http://leon.bottou.org/projects/infimnist
infimnist.tar.gz : wget.check
	wget http://leon.bottou.org/_media/projects/$@

infimnist : infimnist.tar.gz
	tar xvfz $<
	make -C $@

train8m-%.gz: infimnist
	cd infimnist && ./infimnist lab 10000 8109999 | gzip > ../train8m-labels-idx1-ubyte.gz
	cd infimnist && ./infimnist pat 10000 8109999 | gzip > ../train8m-images-idx3-ubyte.gz

train-%.gz: wget.check
	wget http://yann.lecun.com/exdb/mnist/train-$*.gz

t10k-%.gz: wget.check
	wget http://yann.lecun.com/exdb/mnist/t10k-$*.gz

pixelngrams: pixelngrams.cpp

extractpixels: extractpixels.cpp

extractfeatures: extractfeatures.cpp

%png.model: train-labels-idx1-ubyte.gz train-images-idx3-ubyte.gz extractpixels pixelngrams zsh.check perl.check paste.check
	./do-$*png-train

mnist.model: train-labels-idx1-ubyte.gz train-images-idx3-ubyte.gz extractfeatures zsh.check perl.check paste.check
	./do-mnist-train

mnist8m.model: train8m-labels-idx1-ubyte.gz train8m-images-idx3-ubyte.gz extractfeatures zsh.check perl.check paste.check
	./do-mnist8m-train

%png.test.predictions: %png.model t10k-labels-idx1-ubyte.gz t10k-images-idx3-ubyte.gz zsh.check perl.check paste.check
	./do-pixelngram-test $*png

%.test.predictions: %.model t10k-labels-idx1-ubyte.gz t10k-images-idx3-ubyte.gz zsh.check perl.check paste.check
	./do-test $*

CONFUSION='++$$n; 							\
	   ++$$c if $$F[0] != $$F[1];					\
	   ++$$m{"@{[int($$F[0])]}:$$F[1]"}; } { 			\
	   print "$* test errors: $$c out of $$n";			\
	   print "confusion matrix (rows = truth, columns = prediction):"; \
	   foreach $$true (1 .. 10) {					\
	     print join "\t", map { $$m{"$$true:$$_"} || 0 } (1 .. 10);	\
           }'

%.test.confusion: %.test.predictions perl.check
	@perl -lane $(CONFUSION) $*.test.predictions
