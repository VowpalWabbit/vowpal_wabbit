
1) First, in order to create the data sets please download them from the following URL's and save them with the suggested names:

https://www.openml.org/d/1187 --> BNG_wisconsin.csv
https://www.openml.org/d/1190 --> BNG_cpu_act.csv
https://www.openml.org/d/1189 --> BNG_auto_price.csv
https://www.openml.org/d/41540 --> black_friday.csv
https://www.openml.org/d/40753 --> zurich.csv

2) Please use the vowpal_wabbit/utl/continous_action/preprocess_data.ipynb to create preprocessed data.

Note: for the synethic data "ds" you just need to run vowpal_wabbit/utl/continous_action/create_synthetic_data.ipynb instead of the above two steps.

You then need to put the created data sets in the following path:
vowpal_wabbit/test/train-sets/regression

3) In order to save the results please create a folder: "vowpal_wabbit/results" 

4) For running the online algorithms for CATS as well as the comparators and saving the progressive validation results for each data set you can run:
vowpal_wabbit/scripts/online_$name.sh 

for running the CATS offline algorithm and saving the loss estimation in SRM and test error results for each data set you can run:
vowpal_wabbit/offline_$name.sh 

where name = BNG_wisconsin or BNG_cpu_act or BNG_auto_price or black_friday or zurich or ds_5

