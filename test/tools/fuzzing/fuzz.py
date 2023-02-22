import argparse
import csv
import os
from pathlib import Path
import threading
import time
import shlex
import subprocess
import sys
import signal

# Python 3.8+ required


# The code used to find the test binaries are taken from run_tests.py. If any changes occur
# in one, make sure to update the other
def find_in_path(paths, file_matcher, debug_file_name):
    for path in paths:
        absolute_path = os.path.abspath(str(path))
        if os.path.isdir(absolute_path):
            for file in os.listdir(absolute_path):
                absolute_file = os.path.join(absolute_path, file)
                if file_matcher(absolute_file):
                    return absolute_file
        elif os.path.isfile(absolute_path):
            if file_matcher(absolute_path):
                return absolute_path
        else:
            # path does not exist
            continue
    raise ValueError("Couldn't find {}".format(debug_file_name))


def find_or_use_user_supplied_path(
    test_base_ref_dir,
    user_supplied_bin_path,
    search_paths,
    is_correct_bin_func,
    debug_file_name,
):
    if user_supplied_bin_path is None:
        return find_in_path(search_paths, is_correct_bin_func, debug_file_name)
    else:
        if (
            not Path(user_supplied_bin_path).exists()
            or not Path(user_supplied_bin_path).is_file()
        ):
            raise ValueError(
                "Invalid {debug_file_name} binary path: {}".format(
                    (user_supplied_bin_path)
                )
            )
        return user_supplied_bin_path


def find_test_binary(test_base_ref_dir, user_supplied_bin_path):
    vw_search_paths = [
        Path(test_base_ref_dir).joinpath("../../../build/test/tools/fuzzing")
    ]

    def is_test_binary(file_path):
        file_name = os.path.basename(file_path)
        return file_name == "test_initialize"

    return find_or_use_user_supplied_path(
        test_base_ref_dir=test_base_ref_dir,
        user_supplied_bin_path=user_supplied_bin_path,
        search_paths=vw_search_paths,
        is_correct_bin_func=is_test_binary,
        debug_file_name="test_initialize",
    )


def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--afl_dir",
        required=True,
        help="Directory of where the built outputs for google AFL are located",
    )
    parser.add_argument(
        "--test_bin_path",
        help="Specify test binary to use. Otherwise, binary will be searched for in build directory",
    )
    parser.add_argument(
        "-t",
        "--timeout",
        type=int,
        default=60,
        help="Max runtime for each fuzzing test in minutes. 0 will disable the timeout",
    )
    parser.add_argument(
        "-f",
        "--full_tests",
        action="store_true",
        help="Run full tests. This disables the '-d' option in AFL",
    )
    # parser.add_argument(
    #    '-p', '--parallel', type=int, default=1,  help="Number of CPUs to use per test.")
    args = parser.parse_args()

    # Unfortunately, there doesn't appear to be a sane way to run different tests concurrently. Therefore this script will
    # run for (# of tests * timeout) seconds

    script_dir = Path(os.path.dirname(os.path.abspath(__file__)))
    test_bin = find_test_binary(script_dir, args.test_bin_path)
    # AFL requires a separate read-only dir for the models. I believe it defaults to stdin inputs unless you define
    # the '@@' parameter in the command line
    models_dir = os.path.join(script_dir, "models")

    print(f"Using VW binary: {test_bin}")

    try:
        os.mkdir("output")
    except FileExistsError:
        pass

    # Create a temporary working directory for any output files VW produces
    try:
        os.mkdir("tmpwd")
    except FileExistsError:
        pass
    os.chdir("tmpwd")

    with open(os.path.join(script_dir, "model_commands.csv")) as csvfile:
        csv_reader = csv.DictReader(csvfile)
        for row in csv_reader:
            # AFL command and options
            cmd = [
                os.path.join(args.afl_dir, "afl-fuzz"),
                "-i",
                os.path.join(models_dir, row["model_name"]),
                "-o",
                os.path.join("..", "output", row["model_name"]),
            ]
            if args.full_tests is not True:
                cmd.append("-d")

            # VW test binary and options
            cmd.extend([test_bin, row["command"], "--quiet", "--no_stdin", "-i", "@@"])

            timeout = None
            if args.timeout != 0:
                timeout = 60 * args.timeout

            # shlex.join is only available in python 3.8+
            cmd = shlex.join(cmd)
            print(f"running {cmd}")
            p = subprocess.Popen(cmd, shell=True, preexec_fn=os.setsid)
            try:
                p.communicate(timeout=timeout)

                if p.returncode != 0:
                    sys.exit("Fuzzer initialization failed")
            except subprocess.TimeoutExpired:
                # subprocess.kill() doesn't seem to work for some reason
                os.killpg(os.getpgid(p.pid), signal.SIGKILL)
                print("Timeout on  ", cmd)
            except KeyboardInterrupt:
                os.killpg(os.getpgid(p.pid), signal.SIGKILL)
                print("Finished running ", cmd)


if __name__ == "__main__":
    main()
