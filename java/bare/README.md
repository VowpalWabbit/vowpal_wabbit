This Java binding is specifically build to allow efficient integration into Apache Spark through MMLSpark.

Main differences to the "classic" bindings.

# Examples data
- Classic: strings are passed in
- Bare: the caller is expected to already perform the hashing and only passes in the indices & values

# Main loop in the context of multi-pass
- Classic: the command line code and cache file are used
- Bare: the caller is expected to drive the loop. Since data is already in memory in Apache Spark dataframes there is no need to write to a cache file and read again. 

# AllReduce / Spanning tree
- Bare: the spanning tree class is exposed to Java so that the caller can setup the infrastructure.

# Limitations
- Currently the bare binding code only supports single line examples and simple labels as the classification and regression are the first things to be supported on Apache Spark.