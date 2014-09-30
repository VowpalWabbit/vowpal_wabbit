# Handwritten words dataset collected by
# Rob Kassel at MIT Spoken Language Systems Group
# http://ai.stanford.edu/~btaskar/ocr/

VW = ../../vowpalwabbit/vw
VW_OPTS = -b 24 -l 0.02 --nn 40

RM = rm -f

help:
	@echo handwritten words dataset collected by
	@echo Rob Kassel at MIT Spoken Language Systems Group
	@echo http://ai.stanford.edu/~btaskar/ocr/
	@echo $$ make letter.confusion

letter.data.gz:
	wget http://ai.stanford.edu/~btaskar/ocr/letter.data.gz

letter.names:
	wget http://ai.stanford.edu/~btaskar/ocr/letter.names

letter.vw: ocr2vw.py letter.data.gz letter.names
	python $^ $@ $@.test
	cut -d' ' -f1 $@ | sort | uniq -c | sort -n

# category count
CATN = 26

letter.model: letter.vw
	time $(VW) --oaa $(CATN) --final_regressor $@		\
	  --adaptive --invariant --holdout_off			\
	  --loss_function logistic --passes 100			\
	  $(VW_OPTS) --data $< -k --cache_file $<.cache
	$(RM) $<.cache

letter.predictions: letter.model
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

%.confusion: %.predictions
	@perl -lane $(CONFUSION) $< > $@
	@cat $@

clean:
	$(RM) letter.*

.PHONY: clean
