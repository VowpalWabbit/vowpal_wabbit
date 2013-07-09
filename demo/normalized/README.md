normalized online learning demo
-------------------------------

These demos show the ability of the normalized learning rule to adapt to
varying feature scales, relative to an unnormalized learning rule.  For
more details on the normalized learning rule see [the paper](http://arxiv.org/abs/1305.6646).

### Instructions ###

- `make all.results`: eventually produces a nice table outlining performance of normalized adaptive gradient (NAG) vs. unnormalized adaptive gradient (AG) for a variety of data sets.
	- **WARNING**: Please be aware that these demos can be network I/O, disk space, and/or CPU intensive.
	- The complete set of demos can take hours to compute the first time, although the results are cached for subsequent reproduction. 
	- You will see lower progressive loss (regret) for NAG than AG.
	- You will also see that the optimal learning rate eta<sup>*</sup> varies less across datasets for NAG than AG.

#### Details ####

This is organized into individual demos which process a single dataset.  The different individual demos are associated with unique make targets.

An individual demo will download a data set and learn a predictor under two conditions: using the normalized learning rule aka NAG (this the vw default), and using adaptive gradient without normalization aka AG (invoked via vw arguments `--adaptive --invariant`).  For both conditions it will do a hyper-parameter sweep to find the optimal in-hindsight learning rate eta<sup>*</sup>.  Note the context here is online learning, so there is no train/test split; rather what is optimized is progressive loss over the input data.

#### About the datasets ####

Note when data is pre-normalized or otherwise does not exhibit varying scales, the normalized learning rule has essentially no effect.  Therefore the data sets used in this demo have been selected because they exhibit varying scales.  The data set [covertype](http://archive.ics.uci.edu/ml/datasets/Covertype) exemplifies how this arises in practice, as it consists of multiple physical measurements with different units.

#### Just doing one dataset ####

There are individual makefile targets that will just download and compare one dataset.  You can compute a subset using a combination of make targets, e.g., `make only.{covertype,shuttle}`.

Invocation | Dataset | Time | Disk | Network 
--- | --- | --- | --- | ---
`make only.bank` | [bank](http://archive.ics.uci.edu/ml/datasets/Bank+Marketing) | 2 minutes | 1Mb | 1Mb
`make only.census` | [census](http://archive.ics.uci.edu/ml/datasets/Census-Income+%28KDD%29) | 10 minutes | 13Mb | 7Mb
`make only.covertype` | [covertype](http://archive.ics.uci.edu/ml/datasets/Covertype) | 12 minutes | 24Mb | 11Mb
`make only.CTslice` | [CT Slice](http://archive.ics.uci.edu/ml/datasets/Relative+location+of+CT+slices+on+axial+axis) | 30 minutes | 40Mb | 18Mb
`make only.MSD` | [MSD](http://archive.ics.uci.edu/ml/datasets/YearPredictionMSD) | 30 minutes | 500Mb | 256Mb
`make only.shuttle` | [Shuttle](http://archive.ics.uci.edu/ml/datasets/Statlog+%28Shuttle%29) | 1 minute | 600Kb | 300Kb 

