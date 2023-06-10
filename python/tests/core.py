from vw_executor.vw import Vw
from vw_executor.vw_opts import Grid
from numpy.testing import assert_allclose
import pandas as pd
import numpy as np
import pytest
import os
from test_helper import (
    json_to_dict_list,
    dynamic_function_call,
    get_function_object,
)

CURR_DICT = os.path.dirname(os.path.abspath(__file__))
TEST_CONFIG_FILES = ["test_cb.json", "test_regs.json"]
TEST_CONFIG_FILES = [json_to_dict_list(i) for i in TEST_CONFIG_FILES]


def cleanup_data_file():
    script_directory = os.path.dirname(os.path.realpath(__file__))
    # List all files in the directory
    files = os.listdir(script_directory)
    # Iterate over the files and remove the ones with .txt extension
    for file in files:
        if file.endswith(".txt"):
            file_path = os.path.join(script_directory, file)
            os.remove(file_path)


@pytest.fixture
def test_descriptions(request):
    resource = request.param
    yield resource  #
    cleanup_data_file()


def core_test(files, grid, outputs, job_assert, job_assert_args):
    vw = Vw(CURR_DICT + "/.vw_cache", reset=True, handler=None)
    result = vw.train(files, grid, outputs)
    for j in result:
        job_assert(j, **job_assert_args)


@pytest.mark.parametrize("test_descriptions", TEST_CONFIG_FILES, indirect=True)
def test_all(test_descriptions):
    for test_description in test_descriptions:
        mutiply = test_description.get("*", None)
        plus = test_description.get("+", None)
        grid = Grid(test_description["grid"])

        if mutiply:
            grid = grid.__mul__(mutiply)
        if plus:
            grid.__mul__(plus)
        options = Grid(grid)
        data = dynamic_function_call(
            "data_generation",
            test_description["data_func"],
            *test_description["data_func_args"].values(),
        )
        for assert_func in test_description["assert_functions"]:
            assert_job = get_function_object("assert_job", assert_func["assert_func"])
            script_directory = os.path.dirname(os.path.realpath(__file__))
            core_test(
                os.path.join(script_directory, data),
                options,
                test_description["output"],
                assert_job,
                assert_func["assert_func_args"],
            )
