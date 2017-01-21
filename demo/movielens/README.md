Low rank quadratic demo
-------------------------------

This demo shows a low-rank approximation to an interaction design matrix
for the [movielens-1M](http://files.grouplens.org/papers/ml-10m-README.html)
dataset.

### About low-rank interactions ###

In movielens-1M, a user has at most one rating per movie, and therefore
a full interaction design between these two variables (in `vw` syntax:
`-q um`) fundamentally cannot generalize.  Since this situation arises
in recommendation systems, low-rank approximations to interaction terms
rose to prominence in the recommendation community, under the moniker
"matrix factorization".  However, the technique is also appropriate
in non-recommendation settings, e.g., when the interaction between two
high cardinality categorical variables is desired but the available data
is too sparse to learn a full interaction model.

There is a great piece of software called [libfm](http://www.libfm.org/)
whose raison d'etre is to fit low-rank approximations to interaction 
designs, and the main author [Steffen Rendle](http://www.kaggle.com/users/25112/steffen-rendle) does quite well on Kaggle.  Imitation is the best form
of flattery.

### How it works ###

If you have two namespaces `a` and `b`, instead of the full interaction
design enabled by specifying `-q ab`, you can have a rank-k interaction
design by specifying `--lrq abk`.  Additionally specifying `--lrqdropout`
trains with dropout which sometimes works better.  When using dropout the
best performing rank tends to be about twice as big as without dropout.
You might find a bit of `--l2` regularization improves generalization.

### Demo Instructions ###
- `make shootout`: eventually produces four results indicating test MAE (mean absolute error) on movielens-1M for
 - linear: a model without any interactions.  basically this creates a user bias and item bias fit.  this is a surprisingly strong baseline in terms of MAE, but is useless for recommendation as it induces the same item ranking for all users.  It achieves test MAE of 0.731.
 - lrq: the linear model augmented with rank-7 interactions between users and movies, aka, "seven latent factors".  It achieves test MAE of 0.709.  I determined that 7 was the best number to use through experimentation.  The key additional `vw` command-line flags vs. the linear model are `--l2 1.25e-7 --lrq um7`.  Performance is sensitive to the choice of `--l2` regularization strength.
 - lrqdropout: the linear model augmented with rank-14 interactions between users and movies, and trained with dropout.  It achieves test MAE of 0.689.  The key additional `vw` command-line flags vs. the linear model are `--lrq um14 --lrqdropout`.
 - lrqdropouthogwild: same as lrqdropout, but trained in parallel on multiple cores without locking, a la [Niu et. al.](http://www.eecs.berkeley.edu/~brecht/papers/hogwildTR.pdf). Test MAE is nondeterministic but typically equivalent to lrqdropout.  The main purpose of this demo is to instruct on how to achieve lock-free parallel learning.  (Note using the cache and a single training core can be faster than using multiple cores and parsing continuously.  However in some cases data is generated dynamically in such volume that the cache is not practical, thus this technique is helpful.)
- the first time you invoke `make shootout` there is a lot of other output.  invoking it a second time will allow you to just see the cached results.
- `make movie_dendrogram.pdf` will produce a couple of PDFs with hierarchical clustering of the movies based on the latent factors found by `--lrq`. It serves as an example on how to extract the latent factors from an `--invert_hash` file. You will need to zoom in in the large dendrogram to find the movie names.

Details about how `vw` is invoked is in the `Makefile`.
