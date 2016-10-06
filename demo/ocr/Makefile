# Handwritten words dataset collected by
# Rob Kassel at MIT Spoken Language Systems Group
# http://ai.stanford.edu/~btaskar/ocr/

VW = ../../vowpalwabbit/vw
# same settings as do-mnist-train
VW_OPTS = -b 24 -l 0.1 --nn 40

# http://stackoverflow.com/questions/1541844/joining-elements-of-a-list-in-gnu-make
noop=
space = $(noop) $(noop)
VW_RUN = $(subst $(space),_,$(VW_OPTS))

RM = rm -f
# Python > 2.7 is required
PYTHON = python

help:
	@echo handwritten words dataset collected by
	@echo Rob Kassel at MIT Spoken Language Systems Group
	@echo http://ai.stanford.edu/~btaskar/ocr/
	@echo $$ make run

letter.data.gz:
	wget http://ai.stanford.edu/~btaskar/ocr/letter.data.gz

letter.names:
	wget http://ai.stanford.edu/~btaskar/ocr/letter.names

letter.vw: ocr2vw.py letter.data.gz letter.names
	$(PYTHON) $^ $@ $@.test
	cut -d' ' -f1 $@ | sort | uniq -c | sort -n

# category count
CATN = 26

letter.model_$(VW_RUN): letter.vw
	time $(VW) --oaa $(CATN) --final_regressor $@		\
	  --adaptive --invariant --holdout_off			\
	  --loss_function logistic --passes 14			\
	  $(VW_OPTS) --data $< -k --cache_file $<.cache_$(VW_RUN)
	$(RM) $<.cache_$(VW_RUN)

letter.predictions_$(VW_RUN): letter.model_$(VW_RUN)
	time $(VW) --testonly --initial_regressor $< --predictions $@ \
	  --data letter.vw.test

# taken almost verbatim from ../mnist/Makefile
CONFUSION='++$$n; $$p=int($$F[0]); $$l=ord($$F[1])-ord("a")+1;		\
	   ++$$c if $$p != $$l;						\
	   ++$$m{"$$l:$$p"}; } { 					\
	   print "$* test errors: $$c out of $$n = " .			\
		sprintf("%.2f%%",100*$$c/$$n) .				\
	   	"\nconfusion matrix (rows = truth, columns = prediction):"; \
	   foreach $$true (1 .. $(CATN)) {				\
	     print join "\t", map { $$m{"$$true:$$_"} || 0 } (1 .. $(CATN));	\
           }'

%.confusion_$(VW_RUN): %.predictions_$(VW_RUN)
	@perl -lane $(CONFUSION) $< > $@
	@cat $@

run : letter.confusion_$(VW_RUN)

clean:
	$(RM) letter.*

.PHONY: clean run
