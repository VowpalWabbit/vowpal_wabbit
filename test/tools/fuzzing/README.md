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

Once a failing test is fixed, add it to the appropriate model directory to ensure it gets tested going forward.
