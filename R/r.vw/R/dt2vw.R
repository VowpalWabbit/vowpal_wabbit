#'Create a vw data file from a R data.frame object
#'
#'@param data [data.table] data.table format (to be transformed)
#'@param fileName [string] file name of the resulting data in VW-friendly format
#'@param namespaces [list / yaml file] name of each namespace and each variable for each namespace
#'can be a R list, or a YAML file example namespace with the IRIS database:
#'namespaces = list(sepal = list(varName = c('Sepal.Length', 'Sepal.Width'), keepSpace=F),
#'petal = list(varName = c('Petal.Length', 'Petal.Width'), keepSpace=F))
#'this creates 2 namespaces (sepal and petal) containing the variables defined by varName.
#'keepSpace allows to keep or remove spaces in categorical variables
#'example: "FERRARI 4Si" ==> "FERRARI_4Si" with keepSpace = F
#'==> "FERRARI 4Si" with keepSpace = T (interpreted
#'by VW as two distinct categorical variables)
#'@param target [string]  target of the data (target)
#'@param weight [string] weight of each line of the dataset (importance)
#'@param tag [string] tag of each line of the dataset
#'@param hard_parse [bool] if equals true, parses the data more strictly to avoid feeding VW with false categorical
#'variables like '_', or same variables perceived differently like "_var" and "var"
#'@import data.table
#'@export
dt2vw <- function(data, fileName, namespaces = NULL, target, weight = NULL, tag = NULL, hard_parse = FALSE, append = FALSE)
{

  data = setDT(data)

  #change target if its boolean to take values in {-1,1}
  if(is.logical(data[[target]]) || sum(levels(factor(data[[target]])) == levels(factor(c(0,1)))) == 2)
  {
    data[[target]][data[[target]] == TRUE] = 1
    data[[target]][data[[target]] == FALSE] = -1
  }
  
  #if namespaces = NULL, define a unique namespace
  if(is.null(namespaces))
  {
    all_vars = colnames(data)[!colnames(data) %in% c(target, weight, tag)]
    namespaces <- list(A = list(varName = all_vars, keepSpace=FALSE))
  }

  #parse variable names
  specChar = '\\(|\\)|\\||\\:'
  specCharSpace = '\\(|\\)|\\||\\:| '

  parsingNames <- function(x) Reduce(c, lapply(x, function(X) gsub(specCharSpace,'_', X)))

  #parse categorical variables
  parsingVar <- function(x, keepSpace, hard_parse)
  {
    #remove leading and trailing spaces, then remove special characters then remove isolated underscores.
    if(!keepSpace)
      spch = specCharSpace
    else
      spch = specChar

    if(hard_parse)
      gsub('(^_( *|_*)+)|(^_$)|(( *|_*)+_$)|( +_+ +)',' ', gsub(specChar,'_', gsub('(^ +)|( +$)', '',x)))
    else
      gsub(spch, '_', x)
  }

  ###   NAMESPACE LOAD WITH A YAML FILE
  if(typeof(namespaces) == "character" && length(namespaces) == 1 && str_sub(namespaces, -4, -1) == "yaml")
  {
    print("###############  USING YAML FILE FOR LOADING THE NAMESPACES  ###############")
    library(yaml)
    namespaces = yaml.load_file(namespaces)
  }

  ###   AVOIDING DATA FORMAT PROBLEMS
  setnames(data, names(data), parsingNames(names(data)))
  names(namespaces) <- parsingNames(names(namespaces))
  for(x in names(namespaces)) namespaces[[x]]$varName = parsingNames(namespaces[[x]]$varName)
  target = parsingNames(target)
  if(!is.null(tag)) tag = parsingNames(tag)
  if(!is.null(weight)) weight = parsingNames(weight)


  ###   INITIALIZING THE HEADER AND INDEX
  #Header: list of variables'name for each namespace
  #Index: check if the variable is numerical (->TRUE) or categorical (->FALSE)
  Header = list()
  Index = list()

  for(namespaceName in names(namespaces))
  {
    Index[[namespaceName]] = vapply(data[,namespaces[[namespaceName]][['varName']],with=FALSE], is.numeric, logical(1))
    #Header[[namespaceName]][Index[[namespaceName]]] = namespaces[[namespaceName]][['varName']][Index[[namespaceName]]]
    Header[[namespaceName]] = namespaces[[namespaceName]][['varName']]

    ###   ESCAPE THE CATEGORICAL VARIABLES
    if(namespaces[[namespaceName]]$keepSpace)
      Header[[namespaceName]][!Index[[namespaceName]]] = paste0("eval(parse(text = 'parsingVar(",
                                                                Header[[namespaceName]][!Index[[namespaceName]]],
                                                                ", keepSpace = T, hard_parse = hard_parse)'))")
    else
      Header[[namespaceName]][!Index[[namespaceName]]] = paste0("eval(parse(text = 'parsingVar(",
                                                                Header[[namespaceName]][!Index[[namespaceName]]],
                                                                ", keepSpace = F, hard_parse = hard_parse)'))")
  }

  #appending the name of the variable to its value for each categorical variable
  sapply(Index, FUN = function(x){sapply(names(x), FUN = function(y){if(!x[[y]]){
                                                    set(data, i=NULL, y, paste0(y,"_",data[[y]]))
                                                    }})})

  ###   FIRST PART OF THE VW DATA FORMAT: target, weight, tag
  formatDataVW = ''
  argexpr = character(0)

  ### Label can be null, no training is performed
  if(!is.null(target))
  {
    # Both weight and tag are not null
    if(!is.null(weight) && !is.null(tag))
    {
      formatDataVW = '%f %f %s'
      argexpr = paste(target, weight, tag, sep = ', ')
    }
    # Weight is null, tag is not null
    else if(is.null(weight) && !is.null(tag))
    {
      formatDataVW = '%f %s'
      argexpr = paste(target, tag, sep = ', ')
    }
    # Weight is not null, tag is null
    else if(!is.null(weight) && is.null(tag))
    {
      formatDataVW = '%f %f'
      argexpr = paste(target, weight, sep = ', ')
    }
    # We just output target
    else
    {
      formatDataVW = '%f'
      argexpr = target
    }
  }

  ###   ADDING THE FORMAT FOR THE VARIABLES OF EACH NAMESPACE, AND CREATING THE ARGUMENT VECTOR
  for(namespaceName in names(namespaces))
  {
    header = Header[[namespaceName]]
    index = Index[[namespaceName]]
    formatNumeric = paste0(header[index], rep(":%f ", sum(index)), collapse = "")
    formatCategorical = paste0(rep("%s", sum(!index)), collapse = " ")

    formatDataVW = c(formatDataVW, paste0(namespaceName, ' ', formatNumeric, formatCategorical))

    paramexpr = paste0(c(header[index], header[!index] ), collapse=', ')

    argexpr = paste0(c(argexpr, paramexpr), collapse = ', ')
  }

  ###   FULL VW DATA STRING (NOT FORMATTED YET) : (%target %weight |A num1:%f %s |B num2:%f %s)
  if (!is.null(tag))
  {
    formatDataVW = paste0(formatDataVW, collapse = '|')
  }
  else
  {
    formatDataVW = paste0(formatDataVW, collapse = ' |')
  }

  formatDataVW = paste0("sprintf2('", formatDataVW, "',",argexpr, ")")
  ###   FORMATTING USING THE DATA.TABLE DYNAMICS TO OBTAIN THE FINAL VW DATA STRING
  temp = data[, eval(parse(text = formatDataVW))]
  temp = paste0(temp, collapse = '\n')

  ###   WRITING THE DATA TO A FILE
  if(!append)
    con = file(fileName,"w")
  else
    con = file(fileName,"a")
  writeLines(temp,con = con)
  close(con)
}


## Work around the "only 100 arguments are allowed" error
## in base::sprintf(). Only works with 'fmt' of length 1.
## Work around the "only 100 arguments are allowed" error
## in base::sprintf(). Only works with 'fmt' of length 1.
sprintf2 <- function(fmt, ...)
{
  MAX_NVAL <- 99L
  args <- list(...)
  if (length(args) <= MAX_NVAL)
    return(sprintf(fmt, ...))
  stopifnot(length(fmt) == 1L)
  not_a_spec_at <- gregexpr("%%", fmt, fixed=TRUE)[[1L]]
  not_a_spec_at <- c(not_a_spec_at, not_a_spec_at + 1L)
  spec_at <- setdiff(gregexpr("%", fmt, fixed=TRUE)[[1L]], not_a_spec_at)
  nspec <- length(spec_at)
  if (length(args) < nspec)
    stop("too few arguments")
  if (nspec <= MAX_NVAL) {
    break_points <- integer(0)
  } else {
    break_points <- seq(MAX_NVAL + 1L, nspec, by=MAX_NVAL)
  }
  break_from <- c(1L, break_points)
  break_to <- c(break_points - 1L, nspec)
  fmt_break_at <- spec_at[break_points]
  fmt_chunks <- substr(rep.int(fmt, length(fmt_break_at) + 1L),
                       c(1L, fmt_break_at),
                       c(fmt_break_at - 1L, nchar(fmt)))
  ans_chunks <- mapply(
    function(fmt_chunk, from, to)
      do.call(sprintf, c(list(fmt_chunk), args[from:to])),
    fmt_chunks,
    break_from,
    break_to
  )
  paste(apply(ans_chunks,1, paste, collapse = ""), collapse = "\n")
}





