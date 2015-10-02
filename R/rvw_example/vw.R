#'Trains Vowpal Wabbit models from R.
#'This function is fairly simple and extensible to other problems, so far just supports binary classification.
#'Thought to be used in conjuction to perf in order to compute validation metrics on left out datasets.
#'See osmot.cs.cornell.edu/kddcup/software.html for more info about perf.
#'
#'
#'@param training_data
#'@param validation_data
#'@param loss function by default logistic
#'@param model name
#'@param b
#'@param learning_rate
#'@param passes
#'@param l1
#'@param l2
#'@param early_terminate
#'@param interactions Add interaction terms. Can be passed in extra also.
#'@param link_function used to generate predictions
#'@param extra These is where more VW commands can be passed as text
#'@param out_probs filename to write probabilities
#'@param validation_labels file to look for validation data true labels - to compute auc using perf
#'@param verbose mostly used to debug but shows AUC and the vw command used to train the model
#'@param do_evaluation TRUE to compute auc on validation_data. Use FALSE, to just score data
#'@examples
#'# 1. Create a training set (training_data) and validation set (validation_data) in vw format.
#'# 2. Install perf
#'# 3. Create a vector of true labels for the validation dataset, in the [0, 1] range. This is what perf likes.
#'# 4. Run one model with the present code
#'
#'\dontrun{
#' auc = vw(training_data='X_train.vw', validation_data='X_valid.vw',
#'         loss='logistic', model='mdl.vw', b=25, learning_rate=0.5,
#'         passes=20, l1=1e-08, l2=1e-08, early_terminate=2,
#'         interactions=NULL, extra='--stage_poly')
#'}
vw <- function(training_data, validation_data,  model='mdl.vw',
               out_probs='out.txt', validation_labels='valid_labels.txt',
               loss='logistic', b=25,
               learning_rate=0.5, passes=1, l1=NULL, l2=NULL, early_terminate=NULL,
               link_function='--link=logistic', extra=NULL,
               do_evaluation=TRUE, use_perf=TRUE, plot_roc=TRUE, verbose=TRUE){
  
  cmd = sprintf('vw -d %s --loss_function %s -f %s', training_data, loss, model)
  cmd = sprintf('%s --learning_rate=%s --passes %s -c', cmd, learning_rate, passes)
  
  if(!is.null(l1))
    cmd = sprintf('%s --l1 %s', cmd, l1)
  
  if(!is.null(l2))
    cmd = sprintf('%s --l2 %s', cmd, l2)
  
  if(!is.null(b))
    cmd = sprintf('%s -b %s', cmd, b)
  
  if(!is.null(early_terminate))
    cmd = sprintf('%s --early_terminate %s', cmd, early_terminate)
  
  if(!is.null(extra))
    cmd = sprintf('%s %s', cmd, extra)
  
  
  cat('Model parameters\n')
  cat(cmd)
  cat('\n')
  system(cmd)
  system(sprintf('rm %s', out_probs)) # remove probability file if it exists
  
  # vw -d test.txt -t -i model.vw --loss_function=logistic -r raw_predictions.txt
  predict = sprintf('vw -t -i %s -p %s %s -d %s', model, out_probs, link_function, validation_data)
  system(predict)
  
  if(do_evaluation){
    if(use_perf){
      # compute auc using perf
      eval_model = sprintf("perf -ROC -files %s %s | cut -c8-14", validation_labels, out_probs)
      auc = system(eval_model, intern = TRUE)
    } else {
      auc = roc_auc(out_probs, validation_labels, plot_roc, cmd)
    }
  }
  
  if(verbose & do_evaluation){
    cat('Model Parameters\n')
    cat(cmd)
    verbose_log = sprintf('AUC: %s', auc)
    print(verbose_log)
  }
  return(auc)
}

# Reads labels file (from the validation dataset) 
# and probabilities (out_file) from vowpal wabbit
# Also added an option to plot or not the AUC
roc_auc <- function(out_probs, validation_labels, plot_roc, cmd, ...){
  probs = fread(out_probs)[['V1']]
  labels = fread(validation_labels)[['V1']]
  
  if(!identical(length(probs), length(labels)))
    stop('The length of the probabilities and labels is different')
  
  # Fix cmd for adding it in title
  cmd = sapply(strsplit(cmd, '-f'), function(x) paste0(x, collapse='\n'))
  cmd = sapply(strsplit(cmd, '-c'), function(x) paste0(x, collapse='\n'))
  
  # Plot ROC curve and return AUC
  roc = roc(labels, probs, auc=TRUE, print.auc=TRUE, print.thres=TRUE)
  if(plot_roc){
    print(plot.roc(roc, main=cmd, cex.main = 0.5, ...))
  }
  
  auc_value = as.numeric(roc$auc[[1]])
  return(auc_value)
}
