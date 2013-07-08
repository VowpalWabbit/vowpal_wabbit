normalized online learning demo
-------------------------------

These demos show the ability of the normalized learning rule to adapt to
varying feature scales, relative to an unnormalized learning rule.  For
more details on the normalized learning rule see [the paper](http://arxiv.org/abs/1305.6646).

### Instructions ###

- `make all.results`: eventually produces a nice table outlining performance of normalized adaptive gradient (NAG) vs. unnormalized adaptive gradient (AG) for a variety of data sets.
	- **WARNING**: Please be aware that these demos can be network I/O, disk space, and/or CPU intensive.

#### Details ####

This is organized into individual demos which process a single dataset.  The different individual demos are associated with unique make targets.

An individual demo will download a data set and learn a predictor under two conditions: using the normalized learning rule (the vw default), and using adaptive gradient without normalization (`--adaptive --invariant`).  For both conditions it will do a hyper-parameter sweep to find the optimal in-hindsight learning rate.  Note the context here is online learning, so there is no train/test split; rather what is optimized is progressive loss over the input data.

#### About the datasets ####

Note when data is pre-normalized or otherwise does not exhibit varying scales, the normalized learning rule has essentially no effect.  Therefore the data sets used in this demo have been selected because they exhibit varying scales.  The data set [covertype](http://archive.ics.uci.edu/ml/datasets/Covertype) exemplifies how this arises in practice, as it consists of multiple physical measurements with different units.

#### Just doing one dataset ####

There are individual makefile targets that will just download and compare one dataset.

Invocation | Dataset | Time | Disk | Network 
--- | --- | --- | --- | ---
`make only.covertype` | [covertype](http://archive.ics.uci.edu/ml/datasets/Covertype) | 12 minutes | 24Mb | 11Mb

