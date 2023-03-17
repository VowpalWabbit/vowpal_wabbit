Probabilistic Label Tree demo
-----------------------------

This demo presents PLT for applications of logarithmic time multilabel classification.
It uses Mediamill dataset from the [LIBLINEAR datasets repository](https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multilabel.html)
and RCV1-2K, Eurlex-4K, AmazonCat-13K, and Wiki10-31K datasets from the [Extreme Multi-Label (XML) repository](http://manikvarma.org/downloads/XC/XMLRepository.html) 
converted to Vowpal Wabbit format using scripts from [this repository](https://github.com/mwydmuch/datasets4vw).
`plt_demo.py` script trains PLT model, and test prediction with threshold = 0.5 as well as top-5 prediction.
The datasets and paremeters can be easliy edited in the script. The script require VW, Python and wget installed.


## PLT options
```
--plt                       Probabilistic Label Tree with <k> labels
--kary_tree                 use <k>-ary tree. By default = 2 (binary tree), 
                            higher values usually give better results, but increase training time
--threshold                 predict labels with conditional marginal probability greater than <thr> threshold     
--top_k                     predict top-<k> labels instead of labels above threshold                 
```


## Tips for using PLT
PLT accelerates training and prediction for a large number of classes, 
if you have less than 10000 classes, you should probably use OAA.
If you have a huge number of labels and features at the same time, 
you will need as many bits (`-b`) as can afford computationally for the best performance.
You may also consider using `--sgd` instead of default adaptive, normalized, and invariant updates to 
gain more memory for feature weights what may lead to better performance.
If you have many rare labels in your data, you should train with `--holdout_off`, that disables usage of holdout (validation) dataset for early stopping.


## References

A detail description and analysis of PLT can be found in the following article:

Kalina Jasinska, Marek Wydmuch, Krzysztof Dembczynski, Mikhail Kuznetsov, Robert Busa-Fekete:
[*Probabilistic Label Trees for Extreme Multi-label Classification*](https://arxiv.org/abs/2009.11218).

```
@misc{Jasinska-Kobus_et_al_2020,
  title         = {Probabilistic Label Trees for Extreme Multi-label Classification},
  author        = {Kalina Jasinska-Kobus and Marek Wydmuch and Krzysztof Dembczynski and Mikhail Kuznetsov and Robert Busa-Fekete},
  year          = {2020},
  eprint        = {2009.11218},
  archivePrefix = {arXiv},
  primaryClass  = {cs.LG}
}
```

PLT has been introduced in the following article:

Kalina Jasinska, Krzysztof Dembczynski, Robert Busa-Fekete, Karlson Pfannschmidt, Timo Klerx, Eyke Hullermeier:
[*Extreme F-measure Maximization using Sparse Probability Estimates*](http://proceedings.mlr.press/v48/jasinska16.html).
Proceedings of The 33rd International Conference on Machine Learning, PMLR 48:1435-1444, 2016.

```
@inproceedings{Jasinska_et_al_2016,
  title     = {Extreme F-measure Maximization using Sparse Probability Estimates},
  author    = {Kalina Jasinska and Krzysztof Dembczynski and Robert Busa-Fekete and Karlson Pfannschmidt and Timo Klerx and Eyke Hullermeier},
  booktitle = {Proceedings of The 33rd International Conference on Machine Learning},
  pages     = {1435--1444},
  year      = {2016},
  editor    = {Maria Florina Balcan and Kilian Q. Weinberger},
  volume    = {48},
  series    = {Proceedings of Machine Learning Research},
  address   = {New York, New York, USA},
  publisher = {PMLR},
}
```
