SHELL=/bin/zsh

.SECONDARY:

numlabels=105033
bits?=31

odp_train.vw.gz:
	wget --tries=100 -c http://hunch.net/~vw/odp_train.vw.gz

odp_test.vw.gz:
	wget --tries=100 -c http://hunch.net/~vw/odp_test.vw.gz

full_hyper%: full_hyper%.vw odp_test.vw.gz
	time vw -t -i $(word 1,$^) <(zcat $(word 2,$^) | sed 's/|/|b/')

ufull_hyper%.vw: bits=30
ufull_hyper%.vw: odp_train.vw.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; $$depth = int (12 + 6 * rand ());	\
	      1; $$mc = int (40 + 80 * rand ());	\
	      1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
	      1; $$rr = rand () > 0.5 ? 1 : 0;		\
	      1; $$dq = rand () > 0.5 ? "a" : "b";	\
	      1; $$no = rand () > 0.5 ? "0" : "1";	\
	      1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$dq, $$no;' | \
	while read eta depth mc hyper rr dq no;		\
	  do (						\
	    echo $$eta $$depth $$mc $$hyper $$rr $$dq $$no;	\
	    time vw -f $@ --recall_tree $(numlabels) <((zcat $<; zcat $<; zcat $<; zcat $<; zcat $<) | sed 's/|/|b/' | ./minishuf 100000) -b $(bits) --loss_function logistic -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --link glf1 --randomized_routing $$rr --node_only $$no -q '\x88'$$dq; \
	     ) > $@.train 2>&1;				\
	  done

pfull_hyper%.vw: bits=30
pfull_hyper%.vw: odp_train.vw.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; $$depth = int (12 + 6 * rand ());	\
	      1; $$mc = int (40 + 80 * rand ());	\
	      1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
	      1; $$rr = rand () > 0.5 ? 1 : 0;		\
	      1; $$dq = rand () > 0.5 ? "a" : "b";	\
	      1; $$no = rand () > 0.5 ? "0" : "1";	\
	      1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$dq, $$no;' | \
	while read eta depth mc hyper rr dq no;		\
	  do (						\
	    echo $$eta $$depth $$mc $$hyper $$rr $$dq $$no;	\
	    time vw -f $@ --recall_tree $(numlabels) <((zcat $<; zcat $<; zcat $<; zcat $<) | sed 's/|/|b/' | ./minishuf 100000) -b $(bits) --loss_function logistic -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --link glf1 --randomized_routing $$rr --node_only $$no -q '\x88'$$dq; \
	     ) > $@.train 2>&1;				\
	  done

mfull_hyper%.vw: bits=30
mfull_hyper%.vw: odp_train.vw.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; $$depth = int (12 + 6 * rand ());	\
	      1; $$mc = int (40 + 80 * rand ());	\
	      1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
	      1; $$rr = rand () > 0.5 ? 1 : 0;		\
	      1; $$dq = rand () > 0.5 ? "a" : "b";	\
	      1; $$no = rand () > 0.5 ? "0" : "1";	\
	      1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$dq, $$no;' | \
	while read eta depth mc hyper rr dq no;		\
	  do (						\
	    echo $$eta $$depth $$mc $$hyper $$rr $$dq $$no;	\
	    time vw -f $@ --recall_tree $(numlabels) <((zcat $<; zcat $<; zcat $<) | sed 's/|/|b/' | ./minishuf 100000) -b $(bits) --loss_function logistic -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --link glf1 --randomized_routing $$rr --node_only $$no -q '\x88'$$dq; \
	     ) > $@.train 2>&1;				\
	  done

full_hyper%.vw: bits=31
full_hyper%.vw: odp_train.vw.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; $$depth = int (12 + 6 * rand ());	\
	      1; $$mc = int (40 + 80 * rand ());	\
	      1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
	      1; $$rr = rand () > 0.5 ? 1 : 0;		\
	      1; $$dq = rand () > 0.5 ? "a" : "b";	\
	      1; $$no = rand () > 0.5 ? "0" : "1";	\
	      1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$dq, $$no;' | \
	while read eta depth mc hyper rr dq no;		\
	  do (						\
	    echo $$eta $$depth $$mc $$hyper $$rr $$dq $$no;	\
	    time vw -f $@ --recall_tree $(numlabels) <((zcat $<; zcat $<) | sed 's/|/|b/' | ./minishuf 100000) -b $(bits) --loss_function logistic -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --link glf1 --randomized_routing $$rr --node_only $$no -q '\x88'$$dq; \
	     ) > full_hyper$*.train 2>&1;				\
	  done

hyper%: bits=29
hyper%: odp_train.vw.gz
	perl -le 'BEGIN { srand $*; };			\
	      1; $$eta = 0.1 + 3.9 * rand ();		\
	      1; $$depth = int (8 + 6 * rand ());	\
	      1; $$mc = int (40 + 120 * rand ());	\
	      1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
	      1; $$rr = rand () > 0.5 ? 1 : 0;		\
	      1; $$dq = rand () > 0.5 ? "a" : "b";	\
	      1; $$no = rand () > 0.5 ? "0" : "1";	\
	      1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$dq, $$no;' | \
	while read eta depth mc hyper rr dq no;		\
	  do (						\
	    echo $$eta $$depth $$mc $$hyper $$rr $$dq $$no;	\
	    vw --recall_tree $(numlabels) <(zcat $< | sed 's/|/|b/' | ./minishuf 100000) -b $(bits) --loss_function logistic -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --link glf1 --randomized_routing $$rr --node_only $$no -q '\x88'$$dq; \
	     ) > $@ 2>&1;				\
	  done

log_multi: log_multi.vw odp_test.vw.gz
	time vw -t -i $^

log_multi.vw: odp_train.vw.gz
	time vw -f $@ --log_multi $(numlabels) $< -b $(bits) --loss_function logistic -l 1 

recall_tree: recall_tree.vw odp_test.vw.gz
	time vw -t -i $(word 1,$^) <(zcat $(word 2,$^) | sed 's/|/|b/')

recall_tree.vw: odp_train.vw.gz
	time vw -f $@ --recall_tree $(numlabels) <(zcat $< | sed 's/|/|b/' | ./minishuf 100000) -b $(bits) --loss_function logistic -l 1 --node_only 1 -q '\x88'b --max_candidates 400 --max_depth 5

oaahogwild.vw: odp_train.vw.gz
	time ./do-oaa-hogwild <(zcat $< | ./minishuf 100000 | gzip -c) -f $@ --oaa $(numlabels) --oaa_subsample 1000 -b $(bits) --loss_function logistic -l 1 

oaahogwild: oaahogwild.vw odp_test.vw.gz
	time ./do-oaa-hogwild $(word 2,$^) -t -i $(word 1,$^)
