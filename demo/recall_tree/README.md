Recall tree demo
-------------------------------

This demo exercises the recall tree reduction for logarithmic time 
multiclass classification.

The dataset used is [ALOI](http://aloi.science.uva.nl/), which only
has 1000 classes in it.  Due to constant factor overhead, I wouldn't 
recommend using recall tree unless you have at least 10,000 classes;
but it's good to have a demo that runs quickly without a lot of memory.

The demo targets are:
- `make defaults`: trains and tests using what are essentially the defaults for recall tree (see below).  Eventually yields 11.2% test error.
- `make tuned`: trains and tests using hyperparameters found via random probe hyperparameter tuning.  Eventually yields 9.9% test error.
- `make oaa`: trains and tests one-against-all using hyperparameters found via random probe hyperparameter tuning.  Yields 12.1% test error (in less time than recall tree, it turns out).

### Recall Tree for your problem ###

- As indicated, if you have less than 10000 classes, you should probably be using OAA.  To accelerate training you can use negative gradient subsampling and multicore training via hogwild.
- If you're still reading, you have more than 10000 classes.  Therefore you will need many bits in your predictor.  Use as many as you can afford computationally.  
  - If your features are all binary then try disabling normalized updates via `--adaptive --invariant`, which is like having an extra bit in your predictor without the memory cost.
- Logistic loss (`--loss_function logistic`) always works better, as far as I can tell.
- There are extra features, consisting of the identities of the nodes in the routing tree, which are added to the example before passing to the underlying binary classifier.
  - These are located in namespace '\x88'.  So, for example, you can interact them with other namespaces as in this demo (see Makefile: this is what `-q '\x88':` is doing.)
  - For problems with lots of features, interacting with all the path features can be both a computational and statistical drag.  Enabling the option `--node_only` only generates a single feature corresponding to the identity of the leaf node in the routing tree, which can be better under these conditions.
- Typically increasing the number of candidates per leaf `--max_candidates` from the default will improve accuracy at the cost of additional computation.
- Computational overhead increases with deeper trees, but (test) accuracy does not necessarily increase.  Therefore you have to play around with `--max_depth` and `--bern_hyper`.
  - Increasing `--bern_hyper` discourages deeper trees, decreasing it encourages deeper trees.  
  - `--max_depth` is a hard limit.
- Randomized routing `--randomized_routing` is a regularizer we came up with which sometimes works well.  It's worth trying, but note that it will make your loss on the training set appear worse, so you'll need to assess on test data.
