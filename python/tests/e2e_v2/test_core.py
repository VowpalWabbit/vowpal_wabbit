"""
E2E tests for VowpalWabbit using vw_executor.

This module uses pytest parametrize to generate test cases from JSON config files.
Each grid combination becomes a separate test case.
VW training is deferred to test execution time (not import time) to avoid
multiprocessing issues with pybind11 bindings.
"""
from vw_executor.vw import Vw
from vw_executor.vw_opts import Grid, VwOpts
import pytest
import os
import logging
import signal
import sys
from test_helper import (
    json_to_dict_list,
    evaluate_expression,
    copy_file,
    custom_sort,
    get_function_obj_with_dirs,
    datagen_driver,
)
from conftest import STORE_OUTPUT

CURR_DIR = os.path.dirname(os.path.abspath(__file__))
logging.basicConfig(
    format="%(asctime)s - %(levelname)s - %(message)s", level=logging.INFO
)


def get_options(grids, expression):
    """Convert grid config to VW options list."""
    final_variables = {}
    for key in grids:
        final_variables[key] = Grid(grids[key])
    return evaluate_expression(expression, final_variables)


def make_test_id(task_folder, test_name, assert_name, opts):
    """Create a unique test ID from the options."""
    opts_str = "_".join("".join([i for i in str(opts) if i != "-"]).split(" "))
    # Truncate if too long
    if len(opts_str) > 80:
        opts_str = opts_str[:77] + "..."
    return f"{task_folder}_{test_name}_{assert_name}_{opts_str}"


def get_test_configs():
    """Load test configurations and expand grid combinations without running VW."""
    config_files = os.listdir(os.path.join(CURR_DIR, "test_configs"))
    test_cases = []

    for config_file in config_files:
        task_folder = config_file.split(".")[0]
        configs = json_to_dict_list(config_file)

        if not isinstance(configs, list):
            configs = [configs]

        for idx, config in enumerate(configs):
            test_name = config.get('test_name', f'test_{idx}')

            # Compute grid combinations at parametrize time (no VW needed)
            options = get_options(config["grids"], config["grids_expression"])

            for opts in options:
                opts = VwOpts(opts)
                for assert_func in config["assert_functions"]:
                    test_id = make_test_id(
                        task_folder, test_name, assert_func['name'], opts
                    )
                    test_cases.append(pytest.param(
                        task_folder,
                        config,
                        opts,
                        assert_func,
                        id=test_id
                    ))

    return test_cases


def cleanup_data_files(task_folder):
    """Clean up generated data files for a task folder."""
    folder_path = os.path.join(CURR_DIR, task_folder)
    try:
        for file in os.listdir(folder_path):
            if file.endswith(".txt"):
                os.remove(os.path.join(folder_path, file))
    except (FileNotFoundError, OSError):
        pass


def run_vw_training_single(task_folder, config, opts):
    """Run VW training for a single grid point and return the job."""
    package_name = [task_folder + ".", ""]
    package_name = custom_sort(task_folder, package_name)
    package_name.append(".")

    # Generate data
    data_func = get_function_obj_with_dirs(
        package_name,
        "data_generation",
        config["data_func"]["name"],
    )
    scenario_directory = os.path.join(CURR_DIR, task_folder)
    data = datagen_driver(
        scenario_directory, data_func, **config["data_func"]["params"]
    )

    # Run training for this single grid point
    vw = Vw(os.path.join(CURR_DIR, ".vw_cache"), reset=True, handler=None)
    result = vw.train(os.path.join(CURR_DIR, data), Grid([opts]), config["output"])

    return result[0], package_name


class TestTimeout(Exception):
    """Raised when a test exceeds its timeout."""
    pass


def timeout_handler(signum, frame):
    raise TestTimeout("Test exceeded 60 second timeout")


@pytest.mark.parametrize("task_folder,config,opts,assert_func_config", get_test_configs())
def test_vw_scenario(task_folder, config, opts, assert_func_config):
    """
    Run a VW training scenario for a single grid point and verify results.

    VW training happens here at test execution time, not at module import time.
    This avoids multiprocessing/fork issues with pybind11 bindings.
    """
    # Set up timeout (Unix only)
    if sys.platform != 'win32':
        old_handler = signal.signal(signal.SIGALRM, timeout_handler)
        signal.alarm(60)  # 60 second timeout per test

    try:
        # Diagnostic logging
        print(f"  [VW] Starting training: {task_folder} opts={opts}", flush=True)

        # Run VW training for this single grid point
        job, package_name = run_vw_training_single(task_folder, config, opts)

        print(f"  [VW] Training complete, running assertion: {assert_func_config['name']}", flush=True)

        # Get the assertion function
        assert_job = get_function_obj_with_dirs(
            package_name, "assert_job", assert_func_config["name"]
        )

        # Run assertion
        assert_job(job, **assert_func_config["params"])

        print(f"  [VW] Assertion passed", flush=True)

        # Optionally save outputs
        if STORE_OUTPUT:
            test_name = (
                assert_job.__name__
                + "_"
                + "_".join("".join([i for i in str(job.opts) if i != "-"]).split(" "))
            )
            output_dir = os.path.join(CURR_DIR, "output", test_name)
            os.makedirs(output_dir, exist_ok=True)

            fileName = str(list(job.outputs.values())[0][0]).split("/")[-1]
            for key, value in list(job.outputs.items()):
                copy_file(value[0], os.path.join(output_dir, f"{key}_{fileName}"))
            copy_file(
                os.path.join(job.cache.path, "cacheNone/" + fileName),
                os.path.join(output_dir, fileName),
            )
    finally:
        # Cancel timeout
        if sys.platform != 'win32':
            signal.alarm(0)
            signal.signal(signal.SIGALRM, old_handler)
        cleanup_data_files(task_folder)
