#'Trains Vowpal Wabbit models from R.
#'
#'This function is fairly simple and extensible to other problems, so far just supports binary classification.
#'Thought to be used in conjuction to perf in order to compute validation metrics on left out datasets.
#'See osmot.cs.cornell.edu/kddcup/software.html for more info about perf.
#'
#'@param training_data a [data.frame] or path to a vw data file
#'@param validation_data a [data.frame] or path to a vw data file
#'@param model name of the model file
#'@param path_vw_data_train if training_data is a [data.frame], the path to which to save
#'the vw data file. If NULL, the data is stored in a temporary folder and deleted before exiting
#'the function
#'@param path_vw_data_val if validation_data is a [data.frame], the path to which to save
#'the vw data file. If NULL, the data is stored in a temporary folder and deleted before exiting
#'the function
#'@param target if training_data or validation_data is a [data.frame], the name of the variable
#'in the [data.frame] corresponding to the target variable
#'@param namespaces used only if training_data or validation_data is a [data.frame]. See arguments
#' of dt2vw
#'@param weight used only if training_data or validation_data is a [data.frame]. See arguments
#' of dt2vw
#'@param tag used only if training_data or validation_data is a [data.frame]. See arguments
#' of dt2vw
#' @param out_probs path to file where to save the predictions. If NULL, the file is stored in
#' a temporary file then deleted.
#'@param loss loss function. By default logistic.
#'@param b number of bits for the weight vector allocation
#'@param learning_rate
#'@param passes
#'@param l1 l1 regularization
#'@param l2 l2 regularization
#'@param early_terminate
#'@param interactions Add interaction terms. Can be passed in extra also.
#'@param link_function used to generate predictions
#'@param extra These is where more VW commands can be passed as text
#'@param out_probs filename to write probabilities
#'@param validation_labels file to look for validation data true labels - to compute auc using perf
#'or roc_auc() from the R package pROC. If the validation data is a [data.frame] and validation_labels
#'is NULL, the validation labels file is deleted before exiting the function. If validation_labels is not
#'NULL, it indicates the path where validation labels should be stored.
#'@param verbose mostly used to debug but shows AUC and the vw command used to train the model
#'@param keep_preds TRUE (default) to return a vector of the predictions
#'@param do_evaluation TRUE to compute auc on validation_data. Use FALSE, to just score data
#'@param use_perf use perf to compute auc. Otherwise, auc_roc() from the R package pROC is used.
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
#'@import pROC
#'@export
vw <- function(training_data, validation_data,  model='mdl.vw',
               path_vw_data_train = NULL, path_vw_data_val = NULL,
               target = NULL, namespaces = NULL, weight = NULL, tag = NULL,
               out_probs= NULL, validation_labels= NULL,
               loss='logistic', b=25,
               learning_rate=0.5, passes=1, l1=NULL, l2=NULL, early_terminate=NULL,
               link_function='--link=logistic', extra=NULL, keep_preds = TRUE,
               do_evaluation=TRUE, use_perf=TRUE, plot_roc=TRUE, verbose=TRUE){


  ## this should not create an unnecessary copy of the arguments
  data_args = list(train = training_data, val = validation_data)
  path_data_args = list(path_vw_data_train, path_vw_data_val)
  for(i in seq_along(data_args))
  {
    if (inherits(data_args[[i]], "data.frame"))
    {
      if(is.null(target))
        stop(paste0(names(data_args)[i],
                    "data argument: input argument is a data.frame, argument 'target' should be specified "))

      if(! is.character(path_data_args[[i]]))
        path_data_args[[i]] = file.path(tempdir(),paste0(names(data_args)[i],".vw"))

      dt2vw(data = data_args[[i]], fileName = path_data_args[[i]],
            namespaces = namespaces, target = target, weight = weight, tag = tag)
    }
    else
    {
      path_data_args[[i]] = data_args[[i]]
    }

  }


  training_data = path_data_args[[1]]
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

  if(is.null(out_probs))
  {
    out_probs = file.path(tempdir(),"preds.vw")
    del_prob = TRUE
  }
  else
    del_prob = FALSE

  validation_data = path_data_args[[2]]
  predict = sprintf('vw -t -i %s -p %s %s -d %s', model, out_probs, link_function, validation_data)
  system(predict)

  if(do_evaluation){
    if(inherits(data_args[[2]], "data.frame"))
    {
      if(is.null(validation_labels))
      {
        del_val = TRUE
        validation_labels = file.path(tempdir(),"val_labs.vw")
      }
      else
        del_val = FALSE

      write.table(x = data_args[[2]][[target]], file = validation_labels, row.names = FALSE,
                  col.names = FALSE)
    }

    if(use_perf){
      # compute auc using perf
      eval_model = sprintf("perf -ROC -files %s %s | cut -c8-14", validation_labels, out_probs)
      auc = system(eval_model, intern = TRUE)
    } else {
      auc = roc_auc(out_probs, validation_labels, plot_roc, cmd)
    }
  }

  if(verbose && do_evaluation){
    cat('Model Parameters\n')
    cat(cmd)
    verbose_log = sprintf('AUC: %s', auc)
    print(verbose_log)
  }

  if(keep_preds)
    probs = fread(out_probs)[['V1']]
  
  ## delete temporary files
  for(i in seq_along(data_args))
    if(inherits(data_args[[i]], "data.frame"))
      file.remove(path_data_args[[i]])
  if(del_prob)
    file.remove(out_probs)
  if(exists("del_val") && del_val)
    file.remove(validation_labels)

  return(list(auc=auc, preds=probs))
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
  cmd = vapply(strsplit(cmd, '-f'), function(x) paste0(x, collapse='\n'), character(1))
  cmd = vapply(strsplit(cmd, '-c'), function(x) paste0(x, collapse='\n'), character(1))

  # Plot ROC curve and return AUC
  roc = roc(labels, probs, auc=TRUE, print.auc=TRUE, print.thres=TRUE)
  if(plot_roc){
    print(plot.roc(roc, main=cmd, cex.main = 0.5, ...))
  }

  auc_value = as.numeric(roc$auc[[1]])
  return(auc_value)
}
