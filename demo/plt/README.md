Probabilistic Label Tree demo
-------------------------------

This demo presents PLT for applications of logarithmic time multilabel classification.
It uses RCV1-2K, AmazonCat-13K, and Wiki10-31K datasets from the [XML repository](http://manikvarma.org/downloads/XC/XMLRepository.html) 
converted to Vowpal Wabbit format using scripts from [this repository](https://github.com/mwydmuch/datasets4vw).
There are separate scripts for each dataset that download data, train PLT model, and test prediction with threshold = 0.5 as well as top-5 prediction. 
Scripts require VW, Python and wget installed.

```
python rcv1x_2K.py
python amazonCat_13K.py
python wiki10_13K.py
```

## PLT options
```
--plt                       Probabilistic Label Tree with <k> labels
--kary_tree                 use <k>-ary tree. By default = 2 (binary tree)
--threshold                 predict labels with conditional marginal probability greater than <thr> threshold"     
--top_k                     predict top-<k> labels instead of labels above threshold                 
```

## Tips for using PLT
PLT accelerates training and prediction for a large number of classes, 
if you have less than 10000 classes, you should probably use OAA.
If you have a huge number of labels and features at the same time, 
you will need as many bits (`-b`) as can afford computationally for the best performance.
You may also consider using `--sgd` instead of default adaptive, normalized, and invariant updates to 
gain more memory for feature weights what may lead to better performance.
You may also consider using `--holdout_off` if you have many rare labels in your data. 

## References

PLT has been introduced in the following article:

Kalina Jasinska, Krzysztof Dembczynski, Robert Busa-Fekete, Karlson Pfannschmidt, Timo Klerx, Eyke Hullermeier:
[*Extreme F-measure Maximization using Sparse Probability Estimates*](http://proceedings.mlr.press/v48/jasinska16.html).
Proceedings of The 33rd International Conference on Machine Learning, PMLR 48:1435-1444, 2016.

```
@inproceedings{Jasinska_et_al_2016,
  title =   {Extreme F-measure Maximization using Sparse Probability Estimates},
  author =      {Kalina Jasinska and Krzysztof Dembczynski and Robert Busa-Fekete and Karlson Pfannschmidt and Timo Klerx and Eyke Hullermeier},
  booktitle =   {Proceedings of The 33rd International Conference on Machine Learning},
  pages =   {1435--1444},
  year =    {2016},
  editor =      {Maria Florina Balcan and Kilian Q. Weinberger},
  volume =      {48},
  series =      {Proceedings of Machine Learning Research},
  address =     {New York, New York, USA},
  publisher =   {PMLR},
}
```