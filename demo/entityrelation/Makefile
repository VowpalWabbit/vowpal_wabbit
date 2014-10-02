SHELL=/bin/bash
VW=../../vowpalwabbit/vw
search_option=--constraints --search_alpha 1e-8
eval_script=./evaluationER.py

.SECONDARY:

all:
	@cat README.md
clean:
	rm -f *.model *.predictions ER_*.vw *.cache evaluationER.py er.zip *~ 

%.check:
	@test -x "$$(which $*)" || {				\
	  echo "ERROR: you need to install $*" 1>&2;		\
	  exit 1;						\
	}

er.zip: wget.check unzip.check
	@echo "downloading entity relation data ..." 1>&2
	@wget -q http://www.cs.illinois.edu/~kchang10/data/$@ \

ER_%.vw: er.zip
	@echo "preprocessing entity relation data ..." 1>&2
	@unzip -o $<

er.model: ER_train.vw
	@$(VW) -b 24 -c -d $< --search 10 --passes 10 --search_task entity_relation ${search_option} -f er.model

er.test.predictions: ER_test.vw er.model
	@$(VW) -b 24 -c -d $< -t $(search_option) -i er.model -p $@

er.perf: ER_test.vw  er.test.predictions python.check
	@$(eval_script)  $< er.test.predictions



