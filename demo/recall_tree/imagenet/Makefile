SHELL=/bin/zsh

numlabels=21841
bits?=30

.SECONDARY:

training.txt.gz:
	wget --tries=100 -c http://hunch.net/~jl/datasets/imagenet/training.txt.gz 
#	ln -s /data/jcl/training.txt.gz

testing.txt.gz:
	wget --tries=100 -c http://hunch.net/~jl/datasets/imagenet/testing.txt.gz 
#	ln -s /data/jcl/testing.txt.gz

full_oaa_hyper%: full_oaa_hyper%.vw testing.txt.gz
	time ./do-oaa-hogwild $(word 2,$^) -i $< -t 

full_oaa_hyper%.vw: training.txt.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; print join " ", $$eta;' | \
	while read eta ;		\
	  do (						\
	    echo $$eta ;	\
            ./do-oaa-hogwild $< -f $@ --oaa $(numlabels) --oaa_subsample 1000 -b $(bits) --loss_function logistic -l $$eta ; \
	     ) \
	  done #> full_oaa_hyper$*.train 2>&1;		\


oaa_hyper%: training.txt.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; print join " ", $$eta;' | \
	while read eta ;		\
	  do (						\
	    echo $$eta ;	\
            vw <(zcat $< | head -500000) --oaa $(numlabels) --oaa_subsample 1000 -b $(bits) --loss_function logistic -l $$eta ; \
	     ) > $@ 2>&1;				\
	  done

timefufull_hyper%: full_hyper%.vw testing.txt.gz
	(time vw -t -i $< <(zcat $(word 2,$^) | head -10000); time vw -t -i $< <(zcat $(word 2,$^) | head -20000)) > $@ 2>&1

full_hyper%: full_hyper%.vw testing.txt.gz
	(time vw -t -i $(word 1,$^) <(zcat $(word 2,$^) | sed 's/|/|b/')) > $@ 2>&1

full_hyper%.vw: training.txt.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; $$depth = int (11 + 6 * rand ());	\
	      1; $$mc = int (20 + 80 * rand ());	\
	      1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
	      1; $$rr = rand () > 0.5 ? 1 : 0;		\
	      1; $$dq = rand () > 0.5 ? "a" : "b";	\
	      1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$dq;' | \
	while read eta depth mc hyper rr dq;		\
	  do (						\
	    echo $$eta $$depth $$mc $$hyper $$rr $$dq;	\
	    vw -f $@ --recall_tree $(numlabels) <(zcat $< | sed 's/|/|b/') -b $(bits) --loss_function logistic -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --link glf1 --randomized_routing $$rr --node_only 1 -q '\x88'$$dq; \
	     ) > full_hyper$*.train 2>&1;		\
	  done

hyper%: training.txt.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; $$depth = int (11 + 6 * rand ());	\
	      1; $$mc = int (20 + 80 * rand ());	\
	      1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
	      1; $$rr = rand () > 0.5 ? 1 : 0;		\
	      1; $$dq = rand () > 0.5 ? "a" : "b";	\
	      1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$dq;' | \
	while read eta depth mc hyper rr dq;		\
	  do (						\
	    echo $$eta $$depth $$mc $$hyper $$rr $$dq;	\
	    vw --recall_tree $(numlabels) <(zcat $< | sed 's/|/|b/' | head -500000) -b $(bits) --loss_function logistic -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --link glf1 --randomized_routing $$rr --node_only 1 -q '\x88'$$dq; \
	     ) > $@ 2>&1;				\
	  done

log_multi: log_multi.vw testing.txt.gz
	time vw -t -i $^

log_multi.vw: training.txt.gz
	time vw -f $@ --log_multi $(numlabels) $< -b $(bits) --loss_function logistic -l 1 

recall_tree: recall_tree.vw testing.txt.gz
	time vw -t -i $(word 1,$^) <(zcat $(word 2,$^) | sed 's/|/|b/')

recall_tree.vw: training.txt.gz
	time vw -f $@ --recall_tree $(numlabels) <(zcat $< | sed 's/|/|b/') -b $(bits) --loss_function logistic -l 1 --node_only 1 -q '\x88'b #--link glf1 --randomized_routing 1 #$(randomized_routing) 

oaahogwild.vw: training.txt.gz
	time ./do-oaa-hogwild $< -f $@ --oaa $(numlabels) --oaa_subsample 1000 -b $(bits) --loss_function logistic -l 1 

oaahogwild: oaahogwild.vw testing.txt.gz
	(time vw -t -i $^) > $@ 2>&1
	#(time ./do-oaa-hogwild -t -i $^) > $@ 2>&1

timefuoaahogwild: oaahogwild.vw testing.txt.gz
	(time vw -t -i $< <(zcat $(word 2,$^) | head -10000); time vw -t -i $< <(zcat $(word 2,$^) | head -20000)) > $@ 2>&1
