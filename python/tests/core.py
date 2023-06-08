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
    generate_string_combinations,
)

CURR_DICT = os.path.dirname(os.path.abspath(__file__))


def combine_list_cmds_grids(cmds, base_grid):
    list_of_key_val = []
    grids = []
    for key, value in cmds.items():
        value = [i for i in value if i != ""]
        if str(value).isdigit():
            list_of_key_val.append(
                [f" {key} {format(li, '.5f').rstrip('0').rstrip('.') }" for li in value]
            )
        else:
            list_of_key_val.append([f" {key} {li}" for li in value])
    for new_cmd in generate_string_combinations(
        [base_grid["#base"][0]], *list_of_key_val
    ):
        tmp_grid = base_grid.copy()
        tmp_grid["#base"][0] = new_cmd
        grids.append(tmp_grid)
    return grids


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
def test_description(request):
    resource = request.param
    yield resource  #
    cleanup_data_file()


def core_test(files, grid, outputs, job_assert, job_assert_args):
    vw = Vw(CURR_DICT + "/.vw_cache", reset=True, handler=None)
    result = vw.train(files, grid, outputs)
    for j in result:
        job_assert(j, **job_assert_args)


@pytest.mark.parametrize(
    "test_description", json_to_dict_list("test_cb.json"), indirect=True
)
def test_all(test_description):

    mutiply = test_description.get("*", None)
    plus = test_description.get("+", None)

    base_grid = test_description["grid"]
    grids = []
    if mutiply:
        grids = combine_list_cmds_grids(mutiply, base_grid)
    else:
        grids.append(base_grid)

    for grid in grids:
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
                script_directory + data,
                options,
                test_description["output"],
                assert_job,
                assert_func["assert_func_args"],
            )
