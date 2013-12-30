SHELL=/bin/zsh
VW=../../vowpalwabbit/vw

.SECONDARY:

all:
	@cat README.md

shootout: $(foreach what,linear lrq lrqdropout,$(what).print)

clean:
	rm -f $(wildcard *results*) $(wildcard *.vw)

ml-%.zip:
	@echo "downloading movielens $*"
	@wget http://files.grouplens.org/datasets/movielens/ml-$*.zip

ml-%/ratings.dat: ml-%.zip
	@rm -rf ml-$*
	@unzip -qq $<
	@(test -d ml-10M100K && mv -f ml-10M100K ml-10m) || true
	@rm -rf __MACOSX
	@touch ml-$*/*

ml-%.ratings.train.vw: ml-%/ratings.dat
	@echo -n "preprocessing movielens $* ..."
	@./ratings2vw ml-$*.ratings.pre.train.vw ml-$*.ratings.test.vw $<
	@perl -ne 'BEGIN { srand 8675309; }; 		\
	           1; print join "\t", rand (), $$_;' 	\
	      ml-$*.ratings.pre.train.vw | sort -k1 |	\
	      cut -f2- > ml-$*.ratings.train.vw
	@rm -f ml-$*.ratings.pre.train.vw
	@echo " complete"

%.test.vw: %.train.vw
	@true

%.print: %.results
	@printf "%s test MAE is %3.3f\n" $* $$(cat $*.results)

#---------------------------------------------------------------------
#               linear model (no interaction terms)               
#---------------------------------------------------------------------

linear.results: ml-1m.ratings.test.vw ml-1m.ratings.train.vw
	@echo "****************************************************"
	@echo "*   training linear model (no interaction terms)   *"
	@echo "****************************************************"
	@echo
	@${VW} --loss_function quantile -l 1 -b 24 --passes 100 	\
	  -k --cache_file $@.cache -d $(word 2,$+) --holdout_off	\
	  --adaptive --invariant -f $@.model
	@echo "****************************************************"
	@echo "*   testing linear model (no interaction terms)    *"
	@echo "****************************************************"
	@echo
	@${VW} --loss_function quantile -t -i $@.model 			\
	  -d $(word 1,$+) -p						\
	  >(perl -lane '$$s+=abs(($$F[0]-$$F[1])); } { 			\
			1; print $$s/$$.;' > $@)
	@rm -f $@.cache $@.model

#---------------------------------------------------------------------
#               low-rank interaction model (without dropout)
#---------------------------------------------------------------------

lrq.results: ml-1m.ratings.test.vw ml-1m.ratings.train.vw
	@echo "*********************************************************"
	@echo "* training low-rank interaction model (without dropout) *"
	@echo "*                                                       *"
	@echo "* vw --lrq um7 ...                                      *"
	@echo "*********************************************************"
	@echo
	@${VW} --loss_function quantile -l 0.1 -b 24 --passes 100 	\
	  -k --cache_file $@.cache -d $(word 2,$+) --holdout_off	\
	  --l2 1e-6 --lrq um7 --adaptive --invariant -f $@.model
	@echo "********************************************************"
	@echo "* testing low-rank interaction model (without dropout) *"
	@echo "********************************************************"
	@echo
	@${VW} --loss_function quantile -t -i $@.model 			\
	  -d $(word 1,$+) -p						\
	  >(perl -lane '$$s+=abs(($$F[0]-$$F[1])); } { 			\
			1; print $$s/$$.;' > $@)
	@rm -f $@.cache $@.model

#---------------------------------------------------------------------
#               low-rank interaction model (with dropout)
#---------------------------------------------------------------------

lrqdropout.results: ml-1m.ratings.test.vw ml-1m.ratings.train.vw
	@echo "******************************************************"
	@echo "* training low-rank interaction model (with dropout) *"
	@echo "*                                                    *"
	@echo "* vw --lrq um12 --lrqdropout ...                     *"
	@echo "******************************************************"
	@echo
	@${VW} --loss_function quantile -l 1 -b 24 --passes 100 	\
	  -k --cache_file $@.cache -d $(word 2,$+) --holdout_off	\
	  --lrq um12 --lrqdropout --adaptive --invariant -f $@.model
	@echo "*****************************************************"
	@echo "* testing low-rank interaction model (with dropout) *"
	@echo "*****************************************************"
	@echo
	@${VW} --loss_function quantile -t -i $@.model 			\
	  -d $(word 1,$+) -p						\
	  >(perl -lane '$$s+=abs(($$F[0]-$$F[1])); } { 			\
			1; print $$s/$$.;' > $@)
	@rm -f $@.cache $@.model
