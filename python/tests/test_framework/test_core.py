from vw_executor.vw import Vw
from vw_executor.vw_opts import Grid
import pandas as pd
import numpy as np
import pytest
import os
import logging
from test_helper import (
    json_to_dict_list,
    dynamic_function_call,
    get_function_object,
    evaluate_expression,
    variable_mapping,
    copy_file,
)
from conftest import STORE_OUTPUT

CURR_DICT = os.path.dirname(os.path.abspath(__file__))
TEST_CONFIG_FILES_NAME = os.listdir(os.path.join(CURR_DICT, "test_configs"))
TEST_CONFIG_FILES = [json_to_dict_list(i) for i in TEST_CONFIG_FILES_NAME]
GENERATED_TEST_CASES = []
logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s", level=logging.INFO
)


def cleanup_data_file():
    script_directory = os.path.dirname(os.path.realpath(__file__))
    # List all files in the directory
    for name in TEST_CONFIG_FILES_NAME:
        name = name.split(".")[0]
        try:
            files = os.listdir(os.path.join(script_directory, name))
        except:
            return
        # Iterate over the files and remove the ones with .txt extension
        for file in files:
            if file.endswith(".txt"):
                file_path = os.path.join(script_directory + "/" + name, file)
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
        test_name = (
            job_assert.__name__
            + "_"
            + "_".join("".join([i for i in str(j.opts) if i != "-"]).split(" "))
        )
        GENERATED_TEST_CASES.append(
            [lambda: job_assert(j, **job_assert_args), test_name]
        )
        if STORE_OUTPUT:
            if not os.path.exists(CURR_DICT + "/output"):
                os.mkdir(CURR_DICT + "/output")
            if not os.path.exists(CURR_DICT + "/output/" + test_name):
                os.mkdir(CURR_DICT + "/output/" + test_name)
            fileName = str(list(j.outputs.values())[0][0]).split("/")[-1]
            for key, value in list(j.outputs.items()):
                copy_file(
                    value[0],
                    CURR_DICT + "/output/" + test_name + "/" + f"{key}_" + fileName,
                )
            copy_file(
                os.path.join(j.cache.path, "cacheNone/" + fileName),
                CURR_DICT + "/output/" + test_name + "/" + fileName,
            )


def get_options(grids, expression):
    variables = variable_mapping(grids)
    final_variables = {}
    for key in variables:
        final_variables[key] = Grid(variables[key])
    return evaluate_expression(expression, final_variables)


@pytest.mark.usefixtures("test_descriptions", TEST_CONFIG_FILES)
def init_all(test_descriptions):
    for tIndex, tests in enumerate(test_descriptions):
        if type(tests) is not list:
            tests = [tests]
        for test_description in tests:
            options = get_options(
                test_description["grids"], test_description["grids_expression"]
            )
            task_folder = TEST_CONFIG_FILES_NAME[tIndex].split(".")[0]
            package_name = [task_folder + ".", ""]
            for dir in package_name:
                try:
                    data = dynamic_function_call(
                        dir + "data_generation",
                        test_description["data_func"]["name"],
                        *test_description["data_func"]["params"].values(),
                    )
                    if data:
                        break
                except:
                    pass

            for assert_func in test_description["assert_functions"]:
                assert_job = get_function_object("assert_job", assert_func["name"])
                if not assert_job:
                    continue
                script_directory = os.path.dirname(os.path.realpath(__file__))
                core_test(
                    os.path.join(script_directory, data),
                    options,
                    test_description["output"],
                    assert_job,
                    assert_func["params"],
                )


try:
    init_all(TEST_CONFIG_FILES)
    for generated_test_case in GENERATED_TEST_CASES:
        test_name = f"test_{generated_test_case[1]}"
        generated_test_case[0].__name__ = test_name
        globals()[test_name] = generated_test_case[0]
finally:
    cleanup_data_file()
