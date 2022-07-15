# Test

Tests for core VW are broken into several groups:
1. [`core.vwtest.json`](./core.vwtest.json) - contains regression tests mostly centered around running the command line executable, but this also includes script based tests.
2. [`slow.vwtest.json`](./slow.vwtest.json) - contains a couple of python based regression tests that take a while to run so are split from the core set.
3. [`unit_test/`](./unit_test) contains C++ unit tests


## `run_tests.py`
`vwtest.json` format tests can be run with [`run_tests.py`](./run_tests.py). This driver runs each test in its own directory and in parallel, which is why `input_files` and `depends_on` must be specified.

By default [`run_tests.py`](./run_tests.py) will run [`core.vwtest.json`](./core.vwtest.json), unless specified using `--test_spec`.

Note: Due to float prediction you almost always want to run `run_tests.py` with fuzzy matching:
```
run_tests.py --fuzzy_compare
```

## `vwtest.json`
This is a JSON file format which describes inputs, commands and expected output of tests. This format can be understood by `run_tests.py` which is the core driver. The schema is defined [here](./vwtest.schema.json).

If you are using VSCode you should automatically get JSON completion and checking when editing a `*.vwtest.json` file.


## Adding a test
1. Add a new JSON object to the end of [`core.vwtest.json`](./core.vwtest.json).
2. Ensure `id` is unique
3. Specify what to run, the expected outputs, any required files. If another test produces the file which this test depends on (e.g. a model) then add the `id` to `depends_on`.
