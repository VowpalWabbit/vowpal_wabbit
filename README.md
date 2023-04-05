<img src="/logo_assets/vowpal-wabbits-github-logo@3x.png" height="auto" width="100%" alt="Vowpal Wabbit">

[![Linux build status](https://img.shields.io/azure-devops/build/vowpalwabbit/3934113c-9e2b-4dbc-8972-72ab9b9b4342/23?label=Linux%20build&logo=Azure%20Devops)](https://dev.azure.com/vowpalwabbit/Vowpal%20Wabbit/_build/latest?definitionId=23&branchName=master)
[![Windows build status](https://img.shields.io/azure-devops/build/vowpalwabbit/3934113c-9e2b-4dbc-8972-72ab9b9b4342/14?label=Windows%20build&logo=Azure%20Devops)](https://dev.azure.com/vowpalwabbit/Vowpal%20Wabbit/_build/latest?definitionId=14&branchName=master)

[![codecov](https://codecov.io/gh/VowpalWabbit/vowpal_wabbit/branch/master/graph/badge.svg)](https://codecov.io/gh/VowpalWabbit/vowpal_wabbit)
[![Total Alerts](https://img.shields.io/lgtm/alerts/g/JohnLangford/vowpal_wabbit.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/JohnLangford/vowpal_wabbit/alerts/)

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
- [Building](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Building)
- [Tutorial](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Tutorial)
