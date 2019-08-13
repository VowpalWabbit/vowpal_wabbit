<img src="/logo_assets/vowpal-wabbits-github-logo@3x.png" height="auto" width="100%" alt="Vowpal Wabbit">

[![Build Status](https://travis-ci.org/VowpalWabbit/vowpal_wabbit.svg?branch=master)](https://travis-ci.org/VowpalWabbit/vowpal_wabbit)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/6hqpd9e64h72gybr/branch/master?svg=true)](https://ci.appveyor.com/project/JohnLangford/vowpal-wabbit/branch/master)

[![Coverage Status](https://coveralls.io/repos/github/VowpalWabbit/vowpal_wabbit/badge.svg?branch=master)](https://coveralls.io/github/VowpalWabbit/vowpal_wabbit?branch=master)
[![Total Alerts](https://img.shields.io/lgtm/alerts/g/JohnLangford/vowpal_wabbit.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/JohnLangford/vowpal_wabbit/alerts/)
[![Gitter chat](https://badges.gitter.im/VowpalWabbit.png)](https://gitter.im/VowpalWabbit)

This is the *Vowpal Wabbit* fast online learning code.

## Why Vowpal Wabbit?
Vowpal Wabbit is a machine learning system which pushes the frontier of machine learning with techniques such as online, hashing, allreduce, reductions, learning2search, active, and interactive learning. There is a specific focus on reinforcement learning with several contextual bandit algorithms implemented and the online nature lending to the problem well. Vowpal Wabbit is a destination for implementing and maturing state of the art algorithms with performance in mind.

- **Input Format.** The input format for the learning algorithm is substantially more flexible than might be expected. Examples can have features consisting of free form text, which is interpreted in a bag-of-words way. There can even be multiple sets of free form text in different namespaces.
- **Speed.** The learning algorithm is fast -- similar to the few other online algorithm implementations out there. There are several optimization algorithms available with the baseline being sparse gradient descent (GD) on a loss function.
- **Scalability.** This is not the same as fast. Instead, the important characteristic here is that the memory footprint of the program is bounded independent of data. This means the training set is not loaded into main memory before learning starts. In addition, the size of the set of features is bounded independent of the amount of training data using the hashing trick.
- **Feature Interaction.** Subsets of features can be internally paired so that the algorithm is linear in the cross-product of the subsets. This is useful for ranking problems. The alternative of explicitly expanding the features before feeding them into the learning algorithm can be both computation and space intensive, depending on how it's handled.

[Visit the wiki to learn more.](https://github.com/VowpalWabbit/vowpal_wabbit/wiki)

## Getting Started
For the most up to date instructions for getting started on Windows, MacOS or Linux [please see the wiki](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Getting-started). This includes:

- [Installing with a package manager](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Getting-started)
- [Dependencies](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Dependencies)
- [Building](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Building)
- [Tutorial](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Tutorial)

