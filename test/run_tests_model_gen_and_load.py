import argparse
from pathlib import Path
import re
import os
import json
from run_tests import convert_to_test_data, Color, NoColor
import vowpalwabbit


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


def run_command_line(test_id, command, working_dir, color_enum=Color):
    command = command + " --quiet "
    print(f"{color_enum.LIGHT_CYAN}id: {test_id}, command: {command}{color_enum.ENDC}")
    vw = vowpalwabbit.Workspace(command)

    vw.save(f"{working_dir}/model_{test_id}.vw")
    vw.finish()


def load_models(test_id, command, working_dir, color_enum=Color):
    command = command + " --quiet "
    command = command + f" -i {working_dir}/model_{test_id}.vw "

    # link is changed in some reductions so it will clash with saved model
    if "--link" in command:
        command = re.sub("--link [:a-zA-Z0-9_.\\-/]*", "", command)
    # random seed state is stored in the model so it will clash if passed again
    if "--random_seed" in command:
        command = re.sub("--random_seed [0-9]*", "", command)

    print(
        f"{color_enum.LIGHT_PURPLE}id: {test_id}, command: {command}{color_enum.ENDC}"
    )
    vw = vowpalwabbit.Workspace(command)
    vw.finish()


def get_tests(explicit_tests=None):
    test_ref_dir: Path = Path(__file__).resolve().parent

    test_spec_file: Path = test_ref_dir / default_test_file

    test_spec_path = Path(test_spec_file)
    json_test_spec_content = open(test_spec_path).read()
    tests = json.loads(json_test_spec_content)
    print(f"Tests read from file: {test_spec_path.resolve()}")

    tests = convert_to_test_data(
        tests,
        "",
        "",
        [],
        "",
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
            test.command_line = re.sub(
                "--final_regressor [:a-zA-Z0-9_.\\-/]*", "", test.command_line
            )
            test.command_line = test.command_line.replace("--onethread", "")
            filtered_tests.append(test)

    if explicit_tests:
        filtered_tests = list(filter(lambda x: x.id in explicit_tests, filtered_tests))
    return filtered_tests


def generate_all(tests, working_dir, color_enum=Color):
    for test in tests:
        run_command_line(test.id, test.command_line, working_dir, color_enum)

    print(f"stored models in: {working_dir}")


def load_all(tests, working_dir, color_enum=Color):
    if len(os.listdir(working_dir)) != len(tests):
        print(
            f"{color_enum.LIGHT_RED} Warning: There is a mismatch between the number of models in {working_dir} and the number of tests that will attempt to load them {color_enum.ENDC}"
        )

    for test in tests:
        load_models(test.id, test.command_line, working_dir, color_enum)


# will be called when run with pytest
def test_all():
    temp_working_dir = Path.home() / default_working_dir_name
    temp_working_dir.mkdir(parents=True, exist_ok=True)
    tests = get_tests()
    load_all(tests, temp_working_dir)


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
    temp_working_dir.mkdir(parents=True, exist_ok=True)
    if args.clear_working_dir:
        if args.load_models:
            print(
                f"{color_enum.LIGHT_RED} --load_models specified along with --clear_working_dir. Loading models will not work without a directory of stored models. Exiting... {color_enum.ENDC}"
            )
            print(f"{color_enum.LIGHT_RED} Exiting... {color_enum.ENDC}")
            return
        for f in os.scandir(temp_working_dir):
            os.remove(f.path)

    tests = get_tests(args.test)

    if args.generate_models:
        generate_all(tests, temp_working_dir, color_enum)
    elif args.load_models:
        load_all(tests, temp_working_dir, color_enum)
    elif args.generate_and_load:
        generate_all(tests, temp_working_dir, color_enum)
        load_all(tests, temp_working_dir, color_enum)
    else:
        print(
            f"{color_enum.LIGHT_GREEN}Specify a run option, use --help for more info {color_enum.ENDC}"
        )


if __name__ == "__main__":
    main()
