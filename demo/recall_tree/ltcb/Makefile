SHELL=/bin/zsh

.SECONDARY:

numlabels=80000

all: enwik8.train.vw.gz

sweep_oaa%: sweep_oaa%.vw enwik8.test.vw.gz
	(time vw -t -i $(word 1,$^) <(zcat $(word 2,$^) | perl -ne '($$l,$$b)=split /\s/, $$_, 2; $$l = $(numlabels)+2-$$l; $$l = 1+$* if $$l > $*; print "$$l $$b"')) > $@ 2>&1

sweep_oaa%.vw: enwik8.train.vw.gz
	(time vw -f $@ --oaa $$((1+$*)) --oaa_subsample 1000 <(zcat $< | perl -ne '($$l,$$b)=split /\s/, $$_, 2; $$l = $(numlabels)+2-$$l; print "$$l $$b" if $$l <= $*') -q bc --cubic bcd -b 30 --loss_function logistic -l 1) > sweep_oaa$*.train 2>&1

full_inorder_log_multihyper%.vw: bits=32
full_inorder_log_multihyper%.vw: enwik9.inorder.train.vw.gz
	perl -le 'BEGIN { srand $*; };                  \
              1; $$eta = 0.1 + 3.9 * rand ();           \
              1; $$nodes = int (10000 + 40000 * rand ());    	\
	      1; $$loss = rand () > 0.5 ? "squared" : "logistic"; \
              1; $$swap = 2 ** (int (4 * rand ()));	\
              1; $$quad = rand () > 0.5 ? "z" : "b";    \
              1; $$cubic = rand () > 0.5 ? "z" : "b";   \
              1; print join " ", $$eta, $$nodes, $$loss, $$swap, $$quad, $$cubic;' | \
        while read eta nodes loss swap quad cubic;		\
	  do (							\
	    echo $$eta $$nodes $$loss $$swap $$quad $$cubic;	\
	    time vw -f $@ --log_multi $$(( $(numlabels)+1 )) $< -q c$$quad --cubic cd$$cubic -b $(bits) --loss_function $$loss -l $$eta --swap_resistance $$swap; 							\
	  ) > full_inorder_log_multihyper$*.train 2>&1;							\
	  done	  

full_log_multihyper%.vw: bits=32
full_log_multihyper%.vw: enwik9.train.vw.gz
	perl -le 'BEGIN { srand $*; };                  \
              1; $$eta = 0.1 + 3.9 * rand ();           \
              1; $$nodes = int (10000 + 40000 * rand ());    	\
	      1; $$loss = rand () > 0.5 ? "squared" : "logistic"; \
              1; $$swap = 2 ** (int (4 * rand ()));	\
              1; $$quad = rand () > 0.5 ? "z" : "b";    \
              1; $$cubic = rand () > 0.5 ? "z" : "b";   \
              1; print join " ", $$eta, $$nodes, $$loss, $$swap, $$quad, $$cubic;' | \
        while read eta nodes loss swap quad cubic;		\
	  do (							\
	    echo $$eta $$nodes $$loss $$swap $$quad $$cubic;	\
	    time vw -f $@ --log_multi $$(( $(numlabels)+1 )) $< -q c$$quad --cubic cd$$cubic -b $(bits) --loss_function $$loss -l $$eta --swap_resistance $$swap; 							\
	  ) > full_log_multihyper$*.train 2>&1;							\
	  done	  

full_log_multihyper%: full_log_multihyper%.vw enwik9.test.vw.gz
	(time vw -t -i $^) > $@ 2>&1

log_multihyper%: bits=30
log_multihyper%: enwik8.train.vw.gz
	perl -le 'BEGIN { srand $*; };                  \
              1; $$eta = 0.1 + 3.9 * rand ();           \
              1; $$nodes = int (10000 + 40000 * rand ());    	\
	      1; $$loss = rand () > 0.5 ? "squared" : "logistic"; \
              1; $$swap = 2 ** (int (4 * rand ()));	\
              1; $$quad = rand () > 0.5 ? "z" : "b";    \
              1; $$cubic = rand () > 0.5 ? "z" : "b";   \
              1; print join " ", $$eta, $$nodes, $$loss, $$swap, $$quad, $$cubic;' | \
        while read eta nodes loss swap quad cubic;		\
	  do (							\
	    echo $$eta $$nodes $$loss $$swap $$quad $$cubic;	\
	    vw --log_multi $$(( $(numlabels)+1 )) $< -q c$$quad --cubic cd$$cubic -b $(bits) --loss_function $$loss -l $$eta --swap_resistance $$swap; 							\
	  ) > $@ 2>&1;							\
	  done	  

full_inorder_hyper%.vw: enwik9.inorder.train.vw.gz
	perl -le 'BEGIN { srand $*; };                  \
              1; $$eta = 0.1 + 3.9 * rand ();           \
              1; $$depth = int (11 + 6 * rand ());      \
              1; $$mc = int (20 + 80 * rand ());        \
              1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
              1; $$rr = rand () > 0.5 ? 1 : 0;          \
	      1; $$no = rand () > 0.5 ? 1 : 0;		\
              1; $$dq = rand () > 0.5 ? "z" : "b";      \
              1; $$cubic = rand () > 0.5 ? "z" : "b";   \
              1; $$pf = rand () > 0.5 ? "z" : "b";   	\
              1; $$pfc = rand () > 0.5 ? "z" : "b";   	\
              1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$no, $$dq, $$cubic, $$pf, $$pfc;' | \
        while read eta depth mc hyper rr no dq cubic pf pfc; 		\
          do (                                          		\
            echo $$eta $$depth $$mc $$hyper $$rr $$no $$dq $$cubic $$pf $$pfc; \
	    time vw -f $@ $< -b 32 --recall_tree $$(( $(numlabels)+1 )) -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --randomized_routing $$rr --node_only $$no -q c$$dq --cubic cd$$cubic -q '\x88'$$pf --cubic '\x88'c$$pfc; \
	  ) > full_inorder_hyper$*.train 2>&1;					\
	  done	  

full_hyper%: full_hyper%.vw enwik9.test.vw.gz
	(time vw -t -i $^) > $@ 2>&1

full_hyper%.vw: enwik9.train.vw.gz
	perl -le 'BEGIN { srand $*; };                  \
              1; $$eta = 0.1 + 3.9 * rand ();           \
              1; $$depth = int (11 + 6 * rand ());      \
              1; $$mc = int (20 + 80 * rand ());        \
              1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
              1; $$rr = rand () > 0.5 ? 1 : 0;          \
	      1; $$no = rand () > 0.5 ? 1 : 0;		\
              1; $$dq = rand () > 0.5 ? "z" : "b";      \
              1; $$cubic = rand () > 0.5 ? "z" : "b";   \
              1; $$pf = rand () > 0.5 ? "z" : "b";   	\
              1; $$pfc = rand () > 0.5 ? "z" : "b";   	\
              1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$no, $$dq, $$cubic, $$pf, $$pfc;' | \
        while read eta depth mc hyper rr no dq cubic pf pfc; 		\
          do (                                          		\
            echo $$eta $$depth $$mc $$hyper $$rr $$no $$dq $$cubic $$pf $$pfc; \
	    time vw -f $@ $< -b 32 --recall_tree $$(( $(numlabels)+1 )) -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --randomized_routing $$rr --node_only $$no -q c$$dq --cubic cd$$cubic -q '\x88'$$pf --cubic '\x88'c$$pfc; \
	  ) > full_hyper$*.train 2>&1;					\
	  done	  

full_oaa_hyper%: full_oaa_hyper%.vw enwik9.test.vw.gz
	(time vw -t -i $^) > $@ 2>&1

# hogwild fails with more bits ... some kind of deadlock issue :(
full_oaa_hyper%.vw: bits=30
full_oaa_hyper%.vw: enwik9.train.vw.gz
	perl -le 'BEGIN { srand $*; };                  \
              1; $$eta = 0.1 + 3.9 * rand ();           \
              1; print join " ", $$eta;' | \
        while read eta; 		\
          do (                                          		\
            echo $$eta; \
	    unbuffer ./do-oaa-hogwild $< -f $@ --oaa $$(( $(numlabels)+1 )) --oaa_subsample 2000 -q bc --cubic bcd -b $(bits) --loss_function logistic -l $$eta; \
	  ) |& > full_oaa_hyper$*.train 2>&1;           \
	  done	  

oaa_hyper%: bits=30
oaa_hyper%: enwik8.train.vw.gz
	perl -le 'BEGIN { srand $*; };                  \
              1; $$eta = 0.1 + 3.9 * rand ();           \
              1; print join " ", $$eta;' | \
        while read eta; 		\
          do (                                          		\
            echo $$eta; \
	    vw $< --oaa $$(( $(numlabels)+1 )) --oaa_subsample 2000 -q bc --cubic bcd -b $(bits) --loss_function logistic -l $$eta; \
	  ) |& > $@ 2>&1;						\
	  done	  

hyper%: enwik8.train.vw.gz
	perl -le 'BEGIN { srand $*; };                  \
              1; $$eta = 0.1 + 3.9 * rand ();           \
              1; $$depth = int (11 + 6 * rand ());      \
              1; $$mc = int (20 + 80 * rand ());        \
              1; $$log_hyper = log (0.01) + (log (10) - log (0.01)) * rand (); \
              1; $$rr = rand () > 0.5 ? 1 : 0;          \
	      1; $$no = rand () > 0.5 ? 1 : 0;		\
              1; $$dq = rand () > 0.5 ? "z" : "b";      \
              1; $$cubic = rand () > 0.5 ? "z" : "b";   \
              1; $$pf = rand () > 0.5 ? "z" : "b";   	\
              1; $$pfc = rand () > 0.5 ? "z" : "b";   	\
              1; print join " ", $$eta, $$depth, $$mc, exp ($$log_hyper), $$rr, $$no, $$dq, $$cubic, $$pf, $$pfc;' | \
        while read eta depth mc hyper rr no dq cubic pf pfc; 		\
          do (                                          		\
            echo $$eta $$depth $$mc $$hyper $$rr $$no $$dq $$cubic $$pf $$pfc; \
	    vw $< -b 30 --recall_tree $$(( $(numlabels)+1 )) -l $$eta --max_depth $$depth --max_candidates $$mc --bern_hyper $$hyper --randomized_routing $$rr --node_only $$no -q c$$dq --cubic cd$$cubic -q '\x88'$$pf --cubic '\x88'c$$pfc; \
	  ) > $@ 2>&1;							\
	  done	  

sweep_recall_tree%: sweep_recall_tree%.vw enwik8.test.vw.gz
	(time vw -t -i $^) > $@ 2>&1

sweep_recall_tree%.vw: bern_hyper=1
sweep_recall_tree%.vw: enwik8.train.vw.gz
	(time vw -f $@ --recall_tree $$(( $(numlabels)+1 )) $< -q bc --cubic bcd -b 30 --loss_function logistic -l 1 -q '\x88'b --cubic '\x88'bc --max_candidates $* --bern_hyper $(bern_hyper)) > sweep_recall_tree$*.train 2>&1 

recall_tree%: recall_tree%.vw enwik%.test.vw.gz
	vw -t -i $^ 

recall_tree%.vw: vwextra=
recall_tree%.vw: bits=32
recall_tree%.vw: enwik%.train.vw.gz
	vw -f $@ --recall_tree $$(( $(numlabels)+1 )) $< -q bc --cubic bcd -b $(bits) --loss_function logistic -l 1 -q '\x88'b --cubic '\x88'bc $(vwextra) #--link glf1 --randomized_routing $(randomized_routing) 

log_multi%: log_multi%.vw enwik%.test.vw.gz
	time vw -t -i $^ 

log_multi%.vw: enwik%.train.vw.gz
	time vw -f $@ --log_multi $$(( $(numlabels)+1 )) $< -q bc --cubic bcd -b 32 --loss_function logistic 

# NB: 30 instead of 32 here ... hogwild increases memory usage (?)
oaahogwild%.vw: enwik%.train.vw.gz
	./do-oaa-hogwild $< -f $@ --oaa $$(( $(numlabels)+1 )) --oaa_subsample 2000 -q bc --cubic bcd -b 30 --loss_function logistic -l 1 

oaahogwild%: enwik%.test.vw.gz oaahogwild%.vw
	./do-oaa-hogwild $< -i $(word 2,$^) -t

enwik%.zip:
	wget -c http://mattmahoney.net/dc/enwik$*.zip

enwik%.freq: enwik%.zip
	zcat enwik$*.zip | \
	  perl ./wikifil.pl | \
	  perl -lane 'foreach (@F) { $$c{$$_}++; } } \
	           1; { while (($$k, $$v) = each %c) { print "$$k\t$$v"; }' | \
	  sort -k2rn > $@

enwik%.train.vw.gz: enwik%.freq enwik%.zip
	zcat enwik$*.zip | \
	  perl ./wikifil.pl | \
	  perl -ne 'BEGIN { srand 69; }; \
	         1; if (10 * rand () < 1) { print STDERR $$_; \
		 1; } else { print STDOUT $$_; }' \
	  2> >(perl ./gendata.pl enwik$*.freq $(numlabels) 10000000 | gzip > enwik$*.test.vw.gz) \
	  > >(perl ./gendata.pl enwik$*.freq $(numlabels) 10000000 | gzip > enwik$*.train.vw.gz)

# NB: shufbuf size of 1 => in order processing
enwik%.inorder.train.vw.gz: enwik%.freq enwik%.zip
	zcat enwik$*.zip | \
	  perl ./wikifil.pl | \
	  perl -ne 'BEGIN { srand 69; }; \
	         1; if (10 * rand () < 1) { print STDERR $$_; \
		 1; } else { print STDOUT $$_; }' \
	  2> >(perl ./gendata.pl enwik$*.freq $(numlabels) 1 | gzip > enwik$*.inorder.test.vw.gz) \
	  > >(perl ./gendata.pl enwik$*.freq $(numlabels) 1 | gzip > enwik$*.inorder.train.vw.gz)
