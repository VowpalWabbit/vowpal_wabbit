.SECONDARY:

bits?=26
passes?=80

defaults: defaults.vw aloi_test.vw.gz
	zcat $(word 2,$^) | sed 's/|/|f/' | ../../vowpalwabbit/vw -t -i $< 

defaults.vw: aloi_train.vw.gz
	zcat $< | sed 's/|/|f/' | ../../vowpalwabbit/vw -f $@ -k -c --passes $(passes) --recall_tree 1000 -b $(bits) --loss_function logistic -q'\x88'f -l 1

tuned: tuned.vw aloi_test.vw.gz
	zcat $(word 2,$^) | sed 's/|/|f/' | ../../vowpalwabbit/vw -t -i $< 

tuned.vw: aloi_train.vw.gz
	zcat $< | sed 's/|/|f/' | ../../vowpalwabbit/vw -f $@ -k -c --passes $(passes) --recall_tree 1000 -b $(bits) --loss_function logistic -q'\x88'f -l 0.85 --randomized_routing 1 --max_depth 12 --max_candidates 58 --bern_hyper 0.01

aloi_test.vw.gz:
	wget http://lowrank.net/nikos/aloi_test.vw.gz

aloi_train.vw.gz:
	wget http://lowrank.net/nikos/aloi_train.vw.gz
