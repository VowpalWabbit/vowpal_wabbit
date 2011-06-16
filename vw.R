require(digest)

VW_BIN <- '~/vowpal_wabbit/vw'

args2str <- function(args) {

  prepend_dashes <- function(s) if (nchar(s) == 1) paste('-',s,sep="") else paste('--',s,sep="")

  paste(paste(sapply(names(args), prepend_dashes), args), collapse=" ")

}


vw <- function(stdin=NA, args=list(), verbose=T, overwrite=F, remove=F) {
  
  if (is.na(stdin))
    cat('error: please specify stdin, e.g. "cat data.txt"')
  else {
    # convert list of arguments to string
    arg_str <- args2str(args)

    # construct command string: input | ./vw --args
    cmd <- sprintf('%s | %s %s', stdin, VW_BIN, arg_str)
    cat(cmd,'\n')

    # hash command string for an identifier for this job
    cmd_hash <- digest(cmd, 'crc32')
    stdout <- sprintf('vw.%s.stdout', cmd_hash)
    stderr <- sprintf('vw.%s.stderr', cmd_hash)

    if ((file.exists(stdout) || file.exists(stderr)) && !overwrite)
      cat(sprintf('error: log file %s or %s already exists\n', stdout, stderr))
    else {
      # build redirect string for stdout and stderr, and bash string
      redirect_str <- sprintf('>(tee %s) 2> >(tee %s >&2)', stdout, stderr)
      bash_cmd <- sprintf('bash -c "%s > %s"', cmd, redirect_str)

      # touch stdout and stderr, then run vw
      cat(date(), cmd, '\n')
      system(sprintf('touch %s %s', stdout, stderr))
      run_time <- system.time(system(bash_cmd))

      # read results in, parse values returned in vw logs
      # e.g., average loss = 1.13
      result <- readLines(stderr)
      result <- strsplit(result[grep(" = ", result)], ' = ')
      name <- unlist(lapply(result, function (v) v[1]))
      value <- as.numeric(unlist(lapply(result, function (v) v[2])))
      result <- data.frame(row.names=name, value=value)

      # construct final result as a list with reasonable field names
      model <- list(num_examples=result["number of examples",1],
                    num_features=result["total feature number",1],
                    average_loss=result["average loss",1],
                    run_time=run_time)
      
      # rm stdout and stderr files if requested
      if (remove)
        system(sprintf('rm %s %s', stdout, stderr))
    
    }
  }

  model
}


vw_param_sweep <- function(stdin_train, stdin_validate, cache_file, args, base_name="vw", results=data.frame(), results_file=NA) {
  # create all combinations of given input parameters
  sweep_args <- expand.grid(args)

  # convert -q arguments back to characters (will be changed to factors by expand.grid)
  if ("q" %in% names(sweep_args))
    sweep_args$q <- as.character(sweep_args$q)    

  if ("conjugate_gradient" %in% names(sweep_args))
    sweep_args$conjugate_gradient <- as.character(sweep_args$conjugate_gradient)
  
  # read existing results from disk if results_file is given and
  # results data frame is empty
  if (file.exists(results_file) && nrow(results) == 0)
    results <- read.table(results_file, header=T)
  
  for (i in 1:nrow(sweep_args)) {
    # grab this set of parameters
    args <- sweep_args[i,]
    print(args)

    if (!row_match(results, args)) {
    
      # create string for suffix of regressor filename
      arg_str <- paste(paste(names(args), sprintf('%.2g', args), sep='-'), collapse='_')
      reg <- sprintf('%s_%s.reg', base_name, arg_str)

      # list for training arguments
      # add cache file and regressor filename
      args_train <- args
      args_train["f"] <- reg
      args_train["cache_file"] <- cache_file

      # train model
      model_train <- vw(stdin_train, args_train, remove=T)

      # list for validation arguments
      # add regressor filename and testing flag
      args_validate <- args[setdiff(names(args),"passes")]
      args_validate["i"] <- reg
      args_validate["t"] <- " "
      model_validate <- vw(stdin_validate, args_validate, remove=T)

      # append to results data frame
      results <- rbind(results,
                       c(args,
                         train_loss=model_train$average_loss,
                         train_run_time=model_train$run_time["elapsed"],
                         validate_loss=model_validate$average_loss,
                         validate_run_time=model_validate$run_time["elapsed"]))
      
      # write results to file
      if (!is.na(results_file))
        write.table(results, file=results_file, row.names=F)
    }
  }

  results
}

row_match <- function(df, row) {
  if (!all(names(row) %in% names(df)))
    match <- F
  else
    if (nrow(merge(df, row, by=names(row))) > 0)
      match <- T
    else
      match <- F
}
