Recall tree demo
-------------------------------

This demo exercises the recall tree reduction for logarithmic time 
multiclass classification.

The dataset used is [ALOI](http://aloi.science.uva.nl/), which only
has 1000 classes in it.  Due to constant factor overhead, I wouldn't 
recommend using recall tree unless you have at least 10,000 classes;
but it's good to have a demo that runs quickly without a lot of memory.

The demo has two targets:
- `make defaults`: trains and tests using what are essentially the defaults for recall tree (see below).  Eventually yields XXX% test error.
- `make tuned`: trains and tests using hyperparameters found via random probe hyperparameter tuning.  Eventually yields XXX% test error.
- `make oaa`: trains and tests one-against-all using hyperparameters found via random probe hyperparameter tuning.  Eventually yields XXX% test error.
- `make oaahogwild`: the previous, but done using acceleration (negative gradient subsampling and multicore).

### Recall Tree for your problem ###

- As indicated, if you have less than 10000 classes, you should probably be using OAA.  To accelerate training you can use negative gradient subsampling and multicore training via hogwild, in `make oaahogwild`.
- If you're still reading, you have many classes.  Therefore you will need many bits in your predictor.  Use as many as you can afford computationally.  
  - If your features are all binary then try disabling normalized updates via `--adaptive --invariant`, which is like having an extra bit in your predictor without the memory cost.
- Typically increasing the number of candidates per leaf `--max_candidates` from the default will improve accuracy at the cost of additional computation.
- Computational overhead increases with deeper trees, but accuracy does not necessarily increase.  Therefore you have to play around with `--max_depth` and `--bern_hyper`.
  - Increasing `--bern_hyper` discourages deeper trees, decreasing it encourages deeper trees.  `--max_depth` is a hard limit.
- Randomized routing `--randomized_routing` is a regularizer we came up with which sometimes works well.  It's worth trying, but note that it will make your loss on the training set appear worse, so you'll need to assess on test data.
