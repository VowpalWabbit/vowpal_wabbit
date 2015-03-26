Search for dependencies
-------------------------------------

This demo shows the performance of a dependency parser implemented in search
framework. For more details, see

http://arxiv.org/abs/1503.05615

Note that due to the licence issue, we only provide a subset of English Penn 
Treebank. If you have the full set of the data, please change the path in 
Makefile accordingly.


### Instruction ###

- `make dep.perf`: 
	downloads the subset of the English Penn Treebank corpus, trains a dependency parser, and computes test set statistics.

