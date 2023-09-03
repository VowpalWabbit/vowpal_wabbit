# conftest.py
def pytest_addoption(parser):
    parser.addoption(
        "--store_output",
        action="store",
        default=False,
        help="Store output file for tests.",
    )


def pytest_configure(config):
    _store_output = config.getoption("--store_output")
    # Store the custom_arg_value in a global variable or a custom configuration object.
    # For example, you can store it in a global variable like this:
    global STORE_OUTPUT
    STORE_OUTPUT = _store_output
