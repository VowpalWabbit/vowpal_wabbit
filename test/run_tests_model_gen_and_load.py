import argparse
from genericpath import isdir, isfile
from pathlib import Path
import re
import os
import shutil
import json
from run_tests import convert_to_test_data, Color, NoColor, TestData
import vowpalwabbit

from typing import (
    Any,
    Callable,
    Dict,
    List,
    Mapping,
    Optional,
    Set,
    Tuple,
    Type,
    Union,
    cast,
)

"""
This script will use run_tests to capture all available tests to run
Its goal is to generate models (--generate_models) for all tests that are eligible (see get_tests() for the tests that are excluded)
by using the vw python interface and just passing in the command line of the runtest

Most, if not all, runtests have the -d option specified so using the cli directly in the python interface will train a model using the file specified.

After the models are generated we can run the same runtests that will load each model (--load_models) by using the same python interface,
and that will result in the datafile being processed again

The idea is that we can generate the models using one vw version and load the models in an older (or newer) vw version and check for forwards (backwards) model compatibility
"""

default_working_dir_name = ".vw_runtests_model_gen_working_dir"
default_test_file = "core.vwtest.json"


def create_test_dir(
    test_id: int, input_files: List[str], test_base_dir: Path, test_ref_dir: Path
) -> None:
    for f in input_files:
        file_to_copy = None
        search_paths = [test_ref_dir]

        for search_path in search_paths:
            search_path = search_path / f
            if search_path.exists() and not search_path.is_dir():
                file_to_copy = search_path
                break

        if file_to_copy is None:
            raise ValueError(
                f"Input file '{f}' couldn't be found for test {test_id}. Searched in '{test_ref_dir}'"
            )

        test_dest_file = test_base_dir / f
        if file_to_copy == test_dest_file:
            continue
        test_dest_file.parent.mkdir(parents=True, exist_ok=True)
        # We always want to replace this file in case it is the output of another test
        if test_dest_file.exists():
            test_dest_file.unlink()

        shutil.copy(str(file_to_copy), str(test_dest_file))


def generate_model(
    test_id: int,
    command: str,
    working_dir: Path,
    color_enum: Type[Union[Color, NoColor]] = Color,
) -> None:
    command = command + " --quiet "
    print(f"{color_enum.LIGHT_CYAN}id: {test_id}, command: {command}{color_enum.ENDC}")
    vw = vowpalwabbit.Workspace(command)

    vw.save(str(working_dir / f"model_{test_id}.vw"))
    vw.finish()


def load_model(
    test_id: int,
    command: str,
    working_dir: Path,
    color_enum: Type[Union[Color, NoColor]] = Color,
) -> None:
    command = command + " --quiet "
    model_file = str(working_dir / f"model_{test_id}.vw")
    command = command + f" -i {model_file}"

    # link is changed in some reductions so it will clash with saved model
    if "--link" in command:
        command = re.sub("--link [:a-zA-Z0-9_.\\-/]*", "", command)
        command = re.sub("--link=[:a-zA-Z0-9_.\\-/]*", "", command)
    # random seed state is stored in the model so it will clash if passed again
    if "--random_seed" in command:
        command = re.sub("--random_seed [0-9]*", "", command)
        command = re.sub("--random_seed=[0-9]*", "", command)

    print(
        f"{color_enum.LIGHT_PURPLE}id: {test_id}, command: {command}{color_enum.ENDC}"
    )

    try:
        vw = vowpalwabbit.Workspace(command)
        vw.finish()
    except Exception as e:
        print(f"{color_enum.LIGHT_RED} FAILURE!! id: {test_id} {str(e)}")
        raise e


def get_tests(
    working_dir: Path, explicit_tests: Optional[List[int]] = None
) -> List[TestData]:
    test_ref_dir: Path = Path(__file__).resolve().parent

    test_spec_file: Path = test_ref_dir / default_test_file

    test_spec_path = Path(test_spec_file)
    json_test_spec_content = open(test_spec_path).read()
    tests = json.loads(json_test_spec_content)
    print(f"Tests read from file: {test_spec_path.resolve()}")

    tests = convert_to_test_data(
        tests=tests,
        vw_bin="",
        spanning_tree_bin=None,
        skipped_ids=[],
        extra_vw_options="",
    )
    filtered_tests = []
    for test in tests:
        if (
            not test.depends_on
            and not test.is_shell
            and not test.skip
            and not " -i " in test.command_line
            and not "--no_stdin" in test.command_line
            and not "bfgs" in test.command_line
            and not "--flatbuffer" in test.command_line
            and not "--help" in test.command_line
        ):
            test.command_line = re.sub("-f [:a-zA-Z0-9_.\\-/]*", "", test.command_line)
            test.command_line = re.sub("-f=[:a-zA-Z0-9_.\\-/]*", "", test.command_line)
            test.command_line = re.sub(
                "--final_regressor [:a-zA-Z0-9_.\\-/]*", "", test.command_line
            )
            test.command_line = re.sub(
                "--final_regressor=[:a-zA-Z0-9_.\\-/]*", "", test.command_line
            )
            test.command_line = test.command_line.replace("--onethread", "")
            filtered_tests.append(test)

    if explicit_tests:
        filtered_tests = list(filter(lambda x: x.id in explicit_tests, filtered_tests))

    for test in filtered_tests:
        create_test_dir(
            test_id=test.id,
            input_files=test.input_files,
            test_base_dir=working_dir,
            test_ref_dir=test_ref_dir,
        )
    return filtered_tests


def generate_all(
    tests: List[TestData],
    model_working_dir: Path,
    color_enum: Type[Union[Color, NoColor]] = Color,
) -> None:
    os.chdir(model_working_dir.parent)
    for test in tests:
        generate_model(test.id, test.command_line, model_working_dir, color_enum)

    print(f"stored models in: {model_working_dir}")


def load_all(
    tests: List[TestData],
    model_working_dir: Path,
    color_enum: Type[Union[Color, NoColor]] = Color,
) -> None:
    os.chdir(model_working_dir.parent)
    if len(os.listdir(model_working_dir)) != len(tests):
        print(
            f"{color_enum.LIGHT_RED} Warning: There is a mismatch between the number of models in {model_working_dir} and the number of tests that will attempt to load them {color_enum.ENDC}"
        )

    for test in tests:
        load_model(test.id, test.command_line, model_working_dir, color_enum)


def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "-t",
        "--test",
        type=int,
        action="append",
        help="Run specific tests and ignore all others. Can specific as a single integer 'id'",
    )

    parser.add_argument(
        "--generate_models",
        action="store_true",
        help=f"Generate models using all eligible cli's from {default_test_file}",
    )

    parser.add_argument(
        "--load_models",
        action="store_true",
        help=f"Load all models using all eligible cli's from {default_test_file}. Assumes that --generate_models has been run beforehand",
    )

    parser.add_argument(
        "--clear_working_dir",
        action="store_true",
        help=f"Clear all models in {default_working_dir_name}",
    )

    parser.add_argument(
        "--generate_and_load",
        action="store_true",
        help=f"Generate models using all eligible cli's from {default_test_file} and then load them",
    )

    parser.add_argument(
        "--no_color", action="store_true", help="Don't print color ANSI escape codes"
    )

    args = parser.parse_args()
    color_enum = NoColor if args.no_color else Color

    temp_working_dir = Path.home() / default_working_dir_name
    test_model_dir = Path.home() / default_working_dir_name / "test_models"

    if args.clear_working_dir:
        if args.load_models:
            print(
                f"{color_enum.LIGHT_RED} --load_models specified along with --clear_working_dir. Loading models will not work without a directory of stored models. Exiting... {color_enum.ENDC}"
            )
            print(f"{color_enum.LIGHT_RED} Exiting... {color_enum.ENDC}")
        if os.path.isdir(temp_working_dir):
            shutil.rmtree(temp_working_dir)

    else:
        temp_working_dir.mkdir(parents=True, exist_ok=True)
        test_model_dir.mkdir(parents=True, exist_ok=True)
        tests = get_tests(temp_working_dir, args.test)

        if args.generate_models:
            generate_all(tests, test_model_dir, color_enum)
        elif args.load_models:
            load_all(tests, test_model_dir, color_enum)
        elif args.generate_and_load:
            generate_all(tests, test_model_dir, color_enum)
            load_all(tests, test_model_dir, color_enum)
        else:
            print(
                f"{color_enum.LIGHT_GREEN}Specify a run option, use --help for more info {color_enum.ENDC}"
            )


if __name__ == "__main__":
    main()
