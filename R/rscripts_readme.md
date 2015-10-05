### R scripts for vowpal wabbit

### Convert a data.table to vowpal wabbit format
R/dt2vw.R
Allows to convert the data.table in chunks using the append=TRUE option.
Make sure to define the correct data type before using the function. The function
handles different data types as expected from R, so these should be defined 
already in the data.table object.

### Call vowpal wabbit from R
Follow the example in R/rvw_example/vw_example.R. It uses the vw.R function to run VW using system commands, so it is simple to adapt to different models. It also computes the AUC on a validation test set and plots the ROC curve if needed.

