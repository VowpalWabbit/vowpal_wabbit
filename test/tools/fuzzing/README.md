## Building and Running

To use this tool you will need to download and build Google's AFL fuzzer https://github.com/google/AFL

Then run the following commands
```
cd /path/to/vw
mkdir build
cd build
CXX=/path/to/AFL/afl-g++ cmake ..
make test_initialize
```

Then run the fuzz.py script (Python 3.8 or higher is required to run this script)
```
python fuzz.py --afl_dir=/path/to/AFL -t timeout
```

The default timeout is 60 minutes per test, It is not recommended to do much less than that.
Using a timeout of 0 will cause the tests to run to completion or until you cancel the process with C-c (or any keyboard interrupt)

Once a failing test is fixed, add the input model file to the appropriate folder under the `models/` directory to ensure it gets tested going forward.

## Interpreting Output

The results are output into the `output/` directory, which will have the same top level structure as the `models/` directory. Each of these directories contain data from runs based on mutations of the corresponding input directories.

The structure of each directory is as follows
```
+ crashes
+ hangs
+ queue
- fuzz_bitmap
- fuzzer_stats
- plot_data
```

The `crashes` directory contains model files which resulted in a crash.

The `hangs` directory contains model files which resulted in a hang (no end of process detected within a relatively long period of time).

The `queue` directory is a queue of randomly mutated model files which have not yet been run. This allows aborted runs to be resumed.

The model filenames contain the run number and mutation information for reproducibility.

The remaining files contain various statistics about the run, `fuzz_bitmap` particularly is used as an input into an undocumented mode in AFL (https://github.com/google/AFL/blob/61037103ae3722c8060ff7082994836a794f978e/afl-fuzz.c#L7931)