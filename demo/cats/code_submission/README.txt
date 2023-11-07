Platform: Ubuntu

There are two ways to run experiments described in the paper.

A) Use scripts:

1) Ensure that you have all dependencies to build vw.
  https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Dependencies#ubuntu

2) Run run-me.sh.  This will
  a) build vw
  b) download and pre-process existing data sets
  c) generate additional synthetic data sets
  d) run experiments and save results in a results folder


B) Manually go through all the steps:

1) Untar file using the following:
tar -xvf cats.source.tar cats

2) Get vw dependencies and build vw:
cd cats
mkdir build
cd build
cmake ..
make -j vw_cli_bin

3) In order to create the data sets please download them from the following URL's
and save them with the suggested names in the path cats/test/train-sets/regression:

  https://www.openml.org/data/get_csv/150677/BNG_wisconsin.arff       --> BNG_wisconsin.csv
  https://www.openml.org/data/get_csv/150680/BNG_cpu_act.arff         --> BNG_cpu_act.csv
  https://www.openml.org/data/get_csv/150679/BNG_auto_price.arff      --> BNG_auto_price.csv
  https://www.openml.org/data/get_csv/21230845/file639340bd9ca9.arff  --> black_friday.csv
  https://www.openml.org/data/get_csv/5698591/file62a9329beed2.arff   --> zurich.csv

4) Please use the following for each data set to create the preprocessed data.

  sed '/^$/d' -i cats/test/train-sets/regression/$data &
  python3 cats/utl/continous_action/preprocess_data.py -c cats/test/train-sets/regression/$data &

  Note: for the synethic data "ds" instead of the above two steps you need to run the following:

  python3 cats/utl/continous_action/create_synthetic_data.py &

5) In order to save the results please create a folder: "cats/results"

6) For running the online algorithm for CATS as well as the comparators and saving the progressive validation results for each data set you can run:
  run-manual.sh $name on

  for running the CATS offline algorithm and saving the loss estimation in SRM and test error results for each data set you can run:
  run-manual.sh $name off

  where $name = friday, cpu, price, wis, zurich, or ds for different data sets.
