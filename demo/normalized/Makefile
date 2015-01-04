SHELL=/bin/bash
VW=../../vowpalwabbit/vw

.SECONDARY:

all:
	@cat README.md

datasets := bank census covertype CTslice MSD shuttle

all.results.pre: $(foreach what,$(datasets),$(what).best $(what).nonormbest)
	@printf "%20.20s\t%9s\t%9s\t%9s\t%9s\n" "Dataset" "NAG eta^*" "NAG loss" "AG eta^*" "AG loss"

all.results: all.results.pre $(foreach what,$(datasets),$(what).resultsprint)

SHUFFLE='BEGIN { srand 69; };					\
         $$i = int rand 100000;					\
	 print $$b[$$i] if $$b[$$i];				\
	 $$b[$$i] = $$_; } { print grep { defined $$_ } @b;'

#---------------------------------------------------------------------
#                           bank marketing
#
# normalization really helps.  The columns have units of euros, seconds,
# days, and years; in addition there are categorical variables.
#---------------------------------------------------------------------

bank.zip:
	@echo "downloading bank ..." 1>&2
	@wget -q -O - 							     \
	     http://archive.ics.uci.edu/ml/machine-learning-databases/00222/$@ \
	  > $@

bank.preprocess.data.gz: bank.zip
	@echo "preprocessing bank ..." 1>&2
	@zcat $< 2>/dev/null |						\
	  perl -MScalar::Util -F';' -lane '$$l = pop @F; $$p = ($$l =~ /no/) ? -1 : 1; print "$$p $$p|f ", join " ", map { $$isn = Scalar::Util::looks_like_number ($$F[$$_]); $$F[$$_] =~ s/^\s+//; $$F[$$_] =~ s/\s+$$//; $$F[$$_] =~ s/\W/_/g unless $$isn; $$isn ? "@{[$$_+1]}:$$F[$$_]" : "@{[$$_+1]}_$$F[$$_]" } grep { $$F[$$_] =~ /\w/ && ( ! Scalar::Util::looks_like_number ($$F[$$_]) || $$F[$$_] > 0 ) } (0 .. $$#F)' |								\
	perl -e 'BEGIN { srand 69; }; print map { $$_->[1] } sort { $$a->[0] <=> $$b->[0] } map { [ rand (), $$_ ] } <>;' | gzip                        \
	> $@

bank.data: bank.preprocess.data.gz

bank.%.nonormlearn: bank.preprocess.data.gz
	 @${VW} --loss_function logistic -b 22 -q ff -l $* $<		\
	   --adaptive --invariant					\
	   -p >(perl -lane '						\
		  1; ++$$n; $$l+=1.0 if $$F[0] * $$F[1] < 0; } 		\
		  1; { printf "average loss = %f\t%u\t%u\n", $$l/$$n, $$l, $$n;')

bank.%.learn: bank.preprocess.data.gz
	 @${VW} --loss_function logistic -b 22 -q ff -l $* $<		\
	   -p >(perl -lane '						\
		  1; ++$$n; $$l+=1.0 if $$F[0] * $$F[1] < 0; } 		\
		  1; { printf "average loss = %f\t%u\t%u\n", $$l/$$n, $$l, $$n;')

bankbestmin=1e-2
bankbestmax=10
banknonormbestmin=1e-8
banknonormbestmax=1e-3
banktimeestimate=1

#---------------------------------------------------------------------
#                              covertype
#---------------------------------------------------------------------

covtype.data.gz:
	@echo "downloading covertype ..."
	@wget -q -O -                                                    	\
	  http://archive.ics.uci.edu/ml/machine-learning-databases/covtype/$@ 	\
	  > $@

covtype.preprocess.data.gz: covtype.data.gz
	@echo "preprocessing covertype ..." 1>&2
	@perl -F',' 							\
	    -lane '$$l = pop @F; print "$$l $$l|f ", map { "$$_:$$F[$$_] "} grep { $$F[$$_] } (0 .. $$#F)' <(zcat $<) |                  \
	perl -e 'BEGIN { srand 69; }; print map { $$_->[1] } sort { $$a->[0] <=> $$b->[0] } map { [ rand (), $$_ ] } <>;' | gzip                        \
	> $@

covertype.data: covtype.preprocess.data.gz

covertype.%.nonormlearn: covtype.preprocess.data.gz
	${VW} --adaptive --invariant -b 22 --hash all -q ff --oaa 7 -l $* -d $<

covertype.%.learn: covtype.preprocess.data.gz
	${VW} -b 22 --hash all -q ff --oaa 7 -l $* -d $<

covertypebestmin=1e-2
covertypebestmax=10
covertypenonormbestmin=1e-8
covertypenonormbestmax=1e-5
covertypetimeestimate=5

#---------------------------------------------------------------------
#                        million song database
#
# normalization is helpful.
#---------------------------------------------------------------------

YearPredictionMSD.bz2:
	@echo "downloading MSD ..." 1>&2
	@wget -q -O - 								\
	  http://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/regression/$@ 	\
	  > $@

year.preprocess.data.gz: YearPredictionMSD.bz2
	@echo "preprocessing MSD ..." 1>&2
	@perl -lane 'splice @F, 1, 0, "$$F[0]|f"; print join " ", @F' 	\
	  <(bzcat $<) |							\
        perl -ne $(SHUFFLE)						\
	| gzip > $@

MSD.data: year.preprocess.data.gz

MSD.%.learn: year.preprocess.data.gz
	@zcat $< |							\
	perl -lane '$$F[0] -= 1998; print join " ", @F' |		\
	${VW} -q ff -l $* 						\
	   -p >(perl -lane '						\
	   	  1; BEGIN { $$max = 0; $$min = 3000; };		\
		  1; ++$$n; $$l += ($$F[0] + 1998 - $$F[1])**2;  	\
		  1; $$max = $$F[1] if $$max < $$F[1];			\
		  1; $$min = $$F[1] if $$min > $$F[1];			\
		  1; } { $$c = $$l / ($$max - $$min)**2;		\
		  1; printf "average loss = %f\t%.3f\t%.3f\t%.3f\t%.3f\n", $$c/$$n, $$l, $$n, $$min, $$max;')

MSD.%.nonormlearn: year.preprocess.data.gz
	@zcat $< |							\
	perl -lane '$$F[0] -= 1998; print join " ", @F' |		\
	${VW} --adaptive --invariant -q ff -l $* 			\
	   -p >(perl -lane '						\
	   	  1; BEGIN { $$max = 0; $$min = 3000; };		\
		  1; ++$$n; $$l += ($$F[0] + 1998 - $$F[1])**2;  	\
		  1; $$max = $$F[1] if $$max < $$F[1];			\
		  1; $$min = $$F[1] if $$min > $$F[1];			\
		  1; } { $$c = $$l / ($$max - $$min)**2;		\
		  1; printf "average loss = %f\t%.3f\t%.3f\t%.3f\t%.3f\n", $$c/$$n, $$l, $$n, $$min, $$max;')

MSDbestmin=1e-2
MSDbestmax=10
MSDnonormbestmin=1e-8
MSDnonormbestmax=1e-5
MSDtimeestimate=15

#---------------------------------------------------------------------
#                         census-income (KDD)
#---------------------------------------------------------------------

census-income.data.gz:
	@echo "downloading census ..." 1>&2
	@wget -q -O - 							\
	    http://archive.ics.uci.edu/ml/machine-learning-databases/census-income-mld/$@									\
	  > $@

census-income.preprocess.data.gz: census-income.data.gz
	@echo "preprocessing census ..." 1>&2
	@zcat $< |							\
	  perl -MScalar::Util -F',' -lane '$$l = pop @F; $$p = $$l =~ /-/ ? -1 : 1; print "$$p $$p|f ", join " ", map { $$isn = Scalar::Util::looks_like_number ($$F[$$_]); $$F[$$_] =~ s/^\s+//; $$F[$$_] =~ s/\s+$$//; $$F[$$_] =~ s/\W/_/g unless $$isn; $$isn ? "@{[$$_+1]}:$$F[$$_]" : "@{[$$_+1]}_$$F[$$_]" } grep { $$F[$$_] =~ /\w/ && ( ! Scalar::Util::looks_like_number ($$F[$$_]) || $$F[$$_] > 0 ) } (0 .. $$#F)' |								\
	perl -e 'BEGIN { srand 69; }; print map { $$_->[1] } sort { $$a->[0] <=> $$b->[0] } map { [ rand (), $$_ ] } <>;' | gzip                        \
	> $@

census.data: census-income.preprocess.data.gz

census.%.nonormlearn: census-income.preprocess.data.gz
	 @${VW} --hash all --loss_function logistic -q ff -l $* 	\
	   --adaptive --invariant $< 					\
	   -p >(perl -lane '						\
		  1; ++$$n; $$l+=1.0 if $$F[0] * $$F[1] < 0; } 		\
		  1; { printf "average loss = %f\t%u\t%u\n", $$l/$$n, $$l, $$n;')

census.%.learn: census-income.preprocess.data.gz
	 @${VW} --hash all --loss_function logistic -q ff -l $* $<	\
	   -p >(perl -lane '						\
		  1; ++$$n; $$l+=1.0 if $$F[0] * $$F[1] < 0; } 		\
		  1; { printf "average loss = %f\t%u\t%u\n", $$l/$$n, $$l, $$n;')

censusbestmin=1e-2
censusbestmax=10
censusnonormbestmin=1e-8
censusnonormbestmax=1e-4
censustimeestimate=5

#---------------------------------------------------------------------
#                          Statlog (Shuttle)
#---------------------------------------------------------------------

shuttle.trn.Z:
	@echo "downloading Shuttle ..." 1>&2
	@wget -q -O - 							\
	  http://archive.ics.uci.edu/ml/machine-learning-databases/statlog/shuttle/$@									\
	  > $@

shuttle.preprocess.data.gz: shuttle.trn.Z
	@echo "preprocessing Shuttle ..." 1>&2
	@zcat shuttle.trn.Z | 						\
	perl -lane '$$l = pop @F; print "$$l |f ", join " ", map { "@{[$$_+1]}:$$F[$$_]" } grep { $$F[$$_] } (0 .. $$#F)' | 				\
	perl -e 'BEGIN { srand 69; }; print map { $$_->[1] } sort { $$a->[0] <=> $$b->[0] } map { [ rand (), $$_ ] } <>;' | gzip                        \
	 > $@

shuttle.data: shuttle.preprocess.data.gz

shuttle.%.nonormlearn: shuttle.preprocess.data.gz
	${VW} --adaptive --invariant -b 22 --hash all -q ff --oaa 7 -l $* -d $<

shuttle.%.learn: shuttle.preprocess.data.gz
	${VW} -b 22 --hash all -q ff --oaa 7 -l $* -d $<

shuttlebestmin=1e-2
shuttlebestmax=10
shuttlenonormbestmin=1e-8
shuttlenonormbestmax=1e-3
shuttletimeestimate=1

#---------------------------------------------------------------------
#                              CT slices
#
# normalization doesn't help much
#---------------------------------------------------------------------

slice_localization_data.zip:
	@echo "downloading CTslice ..." 1>&2
	@wget -q -O - 							    \
	    http://archive.ics.uci.edu/ml/machine-learning-databases/00206/$@ \
	  > $@

slice_localization.preprocess.data.gz: slice_localization_data.zip
	@echo "preprocessing CTslice ..." 1>&2
	@zcat $< | perl -F',' -lane 'BEGIN { scalar <>; }; shift @F; $$l = pop @F; $$l =~ s/\r//; $$l -= 47; print "$$l $$l|f ", join " ", map { ($$F[$$_] eq "-0.25") ? "$${_}_outside" : "$$_:$$F[$$_]" } grep { $$F[$$_] } (0 .. $$#F)' | perl -e 'BEGIN { srand 69; }; print map { $$_->[1] } sort { $$a->[0] <=> $$b->[0] } map { [ rand (), $$_ ] } <>;' | gzip        	\
	> $@

CTslice.data: slice_localization.preprocess.data.gz

CTslice.%.nonormlearn: slice_localization.preprocess.data.gz
	 @${VW} --adaptive --invariant -b 22 --hash all -q ff -l $* $<  \
	   -p >(perl -lane '						\
	   	  1; BEGIN { $$max = -3000; $$min = 3000; };		\
		  1; ++$$n; $$l += ($$F[0] - $$F[1])**2;  		\
		  1; $$max = $$F[1] if $$max < $$F[1];			\
		  1; $$min = $$F[1] if $$min > $$F[1];			\
		  1; } { $$c = $$l / ($$max - $$min)**2;		\
		  1; printf "average loss = %f\t%.3f\t%.3f\t%.3f\t%.3f\n", $$c/$$n, $$l, $$n, $$min, $$max;')

CTslice.%.learn: slice_localization.preprocess.data.gz
	 @${VW} -b 22 --hash all -q ff -l $* $<				\
	   -p >(perl -lane '						\
	   	  1; BEGIN { $$max = -3000; $$min = 3000; };		\
		  1; ++$$n; $$l += ($$F[0] - $$F[1])**2;  		\
		  1; $$max = $$F[1] if $$max < $$F[1];			\
		  1; $$min = $$F[1] if $$min > $$F[1];			\
		  1; } { $$c = $$l / ($$max - $$min)**2;		\
		  1; printf "average loss = %f\t%.3f\t%.3f\t%.3f\t%.3f\n", $$c/$$n, $$l, $$n, $$min, $$max;')

CTslicebestmin=1e-2
CTslicebestmax=10
CTslicenonormbestmin=1e-5
CTslicenonormbestmax=1
CTslicetimeestimate=15

#---------------------------------------------------------------------
#                           common routines
#---------------------------------------------------------------------

%.best: %.data
	@echo "($*) searching for best in-hindsight learning rate for NAG" 1>&2
	@printf "WARNING: this step takes about %s minutes\n" $($*timeestimate)
	@./hypersearch $($*bestmin) $($*bestmax) '$(MAKE)' '$*.%.learn' > $@

%.nonormbest: %.data
	@echo "($*) searching for best in-hindsight learning rate for AG" 1>&2
	@printf "WARNING: this step takes about %s minutes\n" $($*timeestimate)
	@./hypersearch $($*nonormbestmin) $($*nonormbestmax) '$(MAKE)' '$*.%.nonormlearn' > $@

%.resultsprint:
	@printf "%20.20s\t%9.3g\t%9.3g\t%9.3g\t%9.3g\n" "$*" $$(cut -f1 $*.best) $$(cut -f2 $*.best) $$(cut -f1 $*.nonormbest) $$(cut -f2 $*.nonormbest)

only.%: %.best %.nonormbest all.results.pre %.resultsprint
	@true

.PHONY: all all.results all.results.pre
