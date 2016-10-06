CXXFLAGS=-O3

.SECONDARY:

all:
	@cat README

clean:
	rm -f $(wildcard dna*.test.predictions dna*.model)

%.check:
	@test -x "$$(which $*)" || {				\
	  echo "ERROR: you need to install $*" 1>&2;		\
	  exit 1;						\
	}

dna_train.%.bz2: wget.check
	wget ftp://largescale.ml.tu-berlin.de/largescale/dna/dna_train.$*.bz2

quaddna2vw: quaddna2vw.cpp

%.model: dna_train.lab.bz2 dna_train.dat.bz2 quaddna2vw paste.check bzcat.check zsh.check
	./do-$*-multicore-train

%.test.predictions: %.model dna_train.lab.bz2 dna_train.dat.bz2 quaddna2vw paste.check bzcat.check zsh.check
	./do-test $< $@

%.perf: %.test.predictions perf.check perl.check zsh.check
	./do-perf $<

.PHONY: all clean
