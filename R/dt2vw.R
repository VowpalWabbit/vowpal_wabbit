#data[data.table]:                  data.table format (to be transformed)
#fileName[string]:                  file name of the resulting data in VW-friendly format
#namespaces[list / yaml file]:      name of each namespace and each variable for each namespace
                                    #can be a R list, or a YAML file
#label[string]:                     label of the data (target)
#weight[string]:                    weight of each line of the dataset (importance)
#hard_parse[bool]:                  if equals true, parses the data more strictly to avoid feeding VW with false categorical
                                    #variables like '_', or same variables perceived differently life "_var" and "var" 

dt2vw <- function(data, fileName, namespaces, label, weight, hard_parse = F)
{
  #required packages
  require(data.table)
  
  #parse variable names
  specChar = '\\(|\\)|\\||\\:'
  specCharSpace = '\\(|\\)|\\||\\:| '
  
  parsingNames <- function(x)
  {
    ret = c()
    for(el in x)
      ret = append(ret, gsub(specCharSpace,'_', el))
    ret      
  }
  
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
  label = parsingNames(label)
  if(!is.null(weight)) weight = parsingNames(weight)
  
  
  ###   INITIALIZING THE HEADER AND INDEX 
  #Header: list of variables'name for each namespace
  #Index: check if the variable is numerical (->TRUE) or categorical (->FALSE)
  Header = list()
  Index = list()
  
  for(namespaceName in names(namespaces))
  {
    Index[[namespaceName]] = sapply(data[,namespaces[[namespaceName]][['varName']],with=F], is.numeric)  
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
  
  ###   FIRST PART OF THE VW DATA FORMAT: LABEL AND WEIGHT
  formatDataVW = ''
  argexpr = character(0)  
  if(!is.null(label))
  {
    if(!is.null(weight))
    {
      formatDataVW = '%f %f'
      argexpr = paste(label, weight, sep = ', ')
    }
    else
    {
      formatDataVW = '%f'
      argexpr = label
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
  
  ###   FULL VW DATA STRING (NOT FORMATTED YET) : (%label %weight |A num1:%f %s |B num2:%f %s)
  formatDataVW = paste0(formatDataVW, collapse = ' |')
  formatDataVW = paste0("sprintf('", formatDataVW, "',",argexpr, ")")
  
  ###   FORMATTING USING THE DATA.TABLE DYNAMICS TO OBTAIN THE FINAL VW DATA STRING
  temp = data[, eval(parse(text = formatDataVW))]
  temp = paste0(temp, collapse = '\n')
 
  ###   WRITING THE DATA TO A FILE
  con = file(fileName,"w")
  writeLines(temp,con = con)
  close(con)
}
