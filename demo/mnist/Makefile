.SECONDARY:

help:
	@cat README

%.check:
	@test -x "$$(which $*)" || {				\
	  echo "ERROR: you need to install $*" 1>&2;		\
	  exit 1;						\
	}

testclean:
	rm -f $(wildcard *.test.predictions)

clean:
	rm -f $(wildcard mnist8m.*)

train8m-%.gz: wget.check
	wget http://ml.nec-labs.com/download/data/mnist8m/train8m-$*.gz

t10k-%.gz: wget.check
	wget http://yann.lecun.com/exdb/mnist/t10k-$*.gz

extractfeatures: extractfeatures.cpp

mnist8m.model: train8m-labels-idx1-ubyte.gz train8m-images-idx3-ubyte.gz extractfeatures zsh.check perl.check paste.check
	./do-mnist8m-train

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
