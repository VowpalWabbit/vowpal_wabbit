Training:

../vowpal_wabbit/vowpalwabbit/vw -d dataset.txt --passes (TXM_LEVEL_LIM + 2) -c -k --txm number_of_labels -f dataset_reg.reg -b b_value

TXM_LEVEL_LIM for now is set in txm.h (requires recompiling, will get rid of that) and is the maximal depth of the tree, 1 corresponds to root and two leafs.

In txm.cc there is also TXM_MULTIPLICATIVE_FACTOR (in the learner setup) that one should control.

One chould also control b_value.

Testing:

../vowpal_wabbit/vowpalwabbit/vw -d dataset.txt -t -i dataset_reg.reg