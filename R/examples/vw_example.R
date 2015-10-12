rm(list = ls(all = TRUE)); gc()

# setwd('rvw_example')
# library(ggplot2)
# library(data.table)
# library(pROC)
library(r.vw)

# create a folder called data
system('mkdir data')

#source('dt2vw.R')
#source('rvw_example/vw.R')

# Function used to select variables for each namespace
get_feature_type <- function(X, threshold = 50, verbose = FALSE) {
  q_levels <- function (x)
  {
    if (data.table::is.data.table(x)) {
      unlist(x[, lapply(.SD, function(x) length(unique(x)))])
    }
    else {
      apply(x, 2, function(x) length(unique(x)))
    }
  }

  lvs = q_levels(X)
  fact_vars = names(lvs[lvs < threshold])
  num_vars = names(lvs[lvs >= threshold])
  if (verbose) {
    print(data.frame(lvs))
  }
  list(fact_vars = fact_vars, num_vars = num_vars)
}

# setwd where the data would be
setwd('data')

dt = diamonds
dt = data.table::setDT(dt)
target = 'y'
data_types = get_feature_type(dt[, setdiff(names(dt), target), with=F], threshold = 50)
namespaces = list(n = list(varName = data_types$num_vars, keepSpace=F),
                  c = list(varName = data_types$fact_vars, keepSpace=F))

dt$y = with(dt, ifelse(y < 5.71, 1, -1))
dt2vw(dt, 'diamonds.vw', namespaces, target=target, weight=NULL)
system('head -3 diamonds.vw')

# prepare dataset for validation
system('head -10000 diamonds.vw > X_train.vw ')
system('tail -43940 diamonds.vw > X_valid.vw ')
write.table(tail(dt$y,43940), file='valid_labels.txt', row.names = F,
            col.names = F, quote = F)

training_data='X_train.vw'
validation_data='X_valid.vw'
validation_labels = "valid_labels.txt"
out_probs = "out.txt"
model = "mdl.vw"

# AUC using perf - Download at: osmot.cs.cornell.edu/kddcup/software.html
# It may not work, so a dependency of an R library has been added. See below.
# Commented as could not work.
# auc = vw(training_data, validation_data, loss = "logistic",
#        model, b = 25, learning_rate = 0.5, passes = 1,
#        l1 = NULL, l2 = NULL, early_terminate = NULL,
#        link_function = "--link=logistic", extra = NULL, out_probs = "out.txt",
#        validation_labels = validation_labels, verbose = TRUE, do_evaluation = TRUE)

# Shows files in the working directory: /data
list.files()

auc = vw(training_data, validation_data, loss = "logistic",
         model, b = 25, learning_rate = 0.5, passes = 1,
         l1 = NULL, l2 = NULL, early_terminate = NULL,
         link_function = "--link=logistic", extra = NULL, out_probs = "out.txt",
         validation_labels = validation_labels, verbose = TRUE, do_evaluation = TRUE,
         use_perf=FALSE, plot_roc=TRUE)

print(auc)
# [1] 0.9944229

# AUC using pROC - Saving plots to disk
### create a parameter grid
grid = expand.grid(l1=c(1e-07, 1e-08),
                   l2=c(1e-07, 1e-08),
                   eta=c(0.1, 0.05),
                   extra=c('--nn 10', ''))


cat('Running grid search\n')
pdf('ROCs.pdf')
aucs <- lapply(1:nrow(grid), function(i){
  g = grid[i, ]
  auc = vw(training_data=training_data, # files relative paths
           validation_data=validation_data,
           validation_labels=validation_labels, model=model,
           # grid options
           loss='logistic', b=25, learning_rate=g[['eta']],
           passes=2, l1=g[['l1']], l2=g[['l2']],
           early_terminate=2, extra=g[['extra']],
           # ROC-AUC related options
           use_perf=FALSE, plot_roc=TRUE,
           do_evaluation = TRUE # If false doesn't compute AUC, use only for prediction
           )
  auc
})
dev.off()

results = cbind(iter=1:nrow(grid), grid, auc=do.call(rbind, aucs))
print(results)
# l1    l2  eta   extra     auc
# 1  1e-07 1e-07 0.10 --nn 10 0.9964736
# 2  1e-08 1e-07 0.10 --nn 10 0.9964945
# 3  1e-07 1e-08 0.10 --nn 10 0.9964736
# 4  1e-08 1e-08 0.10 --nn 10 0.9964946
# 5  1e-07 1e-07 0.05 --nn 10 0.9956487
# 6  1e-08 1e-07 0.05 --nn 10 0.9956629
# 7  1e-07 1e-08 0.05 --nn 10 0.9956487
# 8  1e-08 1e-08 0.05 --nn 10 0.9956629
# 9  1e-07 1e-07 0.10         0.9878654
# 10 1e-08 1e-07 0.10         0.9919489
# 11 1e-07 1e-08 0.10         0.9878646
# 12 1e-08 1e-08 0.10         0.9919487
# 13 1e-07 1e-07 0.05         0.9883343
# 14 1e-08 1e-07 0.05         0.9915172
# 15 1e-07 1e-08 0.05         0.9883339
# 16 1e-08 1e-08 0.05         0.9915170

p = ggplot(results, aes(iter, auc, color=extra)) +
  geom_point(size=3) +
  theme_bw() +
  labs(list(x='Iteration', y='AUC',
            title='Logistic regression results'))

print(p)
ggsave('results_plot.png', plot=p)

