### R scripts for vowpal wabbit

### download the r.vw package
r.vw contains two functions: dt2vw and vw  
 
installation requires devtools:
```
install.packages("devtools")
devtools::install_github("JohnLangford/vowpal_wabbit", subdir = "R/r.vw")
```


### Convert a data.table to vowpal wabbit format: dt2vw()
Allows to convert the data.table in chunks using the append=TRUE option.
Make sure to define the correct data type before using the function. The function
handles different data types as expected from R, so these should be defined 
already in the data.table object.

### Call vowpal wabbit from R: vw()
Follow the example in R/examples/vw_example.R and R/examples/vw_example_2.R. It uses the vw.R function to run VW using system commands, so it is simple to adapt to different models. It also computes the AUC on a validation test set and plots the ROC curve if needed.

