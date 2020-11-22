#### Running flatbuffer tests

Note: **Work in Progress** (Not all tests run as of now.)

To run all tests in RunTests for flatbuffers, first all flatbuffer data files need to be created. 
For this, just replacing `../../build/vowpalwabbit/vw` with `../../build/utl/flatbuffer/to_flatbuff` 
would work, provided some flags are removed. This includes `skips`, `ngrams`, etc. A list of 
these is present in `create_bash_script.py` which is used to create the file `create_datafiles.sh`.

The shell script copies other important files like `.stderr`, `.predict` files. Also, the `ref` 
directories, and other files like `dictionary_test`, etc.

To create the datafiles:
- Shell script `bash create_datafiles.sh`

Finally run all possible tests in the usual RunTests format
- RunTests `./RunTests -D -f -E 0.001`