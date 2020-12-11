import shutil
import threading
import argparse
import difflib
from pathlib import Path
import re
import os
import errno
import subprocess
import sys
import hashlib
import traceback

from typing import Optional
import asyncio
import json
import os.path
from concurrent.futures import ThreadPoolExecutor
from enum import Enum
import socket

import runtests_parser

class Color():
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


class Result(Enum):
    SUCCESS = 1
    FAIL = 2
    SKIPPED = 3


def try_decode(binary_object: Optional[bytes]) -> Optional[str]:
    return binary_object.decode("utf-8") if binary_object is not None else ""


# Returns true if they are close enough to be considered equal.
def fuzzy_float_compare(f1, f2, epsilon):
    f1 = float(f1)
    f2 = float(f2)
    delta = abs(f1 - f2)
    if delta < epsilon:
        return True

    # Large number comparison code migrated from Perl RunTests

    # We have a 'big enough' difference, but this difference
    # may still not be meaningful in all contexts. Big numbers should be compared by ratio rather than
    # by difference

    # Must ensure we can divide (avoid div-by-0)
    # If numbers are so small (close to zero),
    # ($delta > $Epsilon) suffices for deciding that
    # the numbers are meaningfully different
    if (abs(f2) <= 1.0):
        return False

    # Now we can safely divide (since abs($word2) > 0) and determine the ratio difference from 1.0
    ratio_delta = abs(f1/f2 - 1.0)
    return ratio_delta < epsilon


def find_in_path(paths, file_matcher):
    for path in paths:
        absolute_path = os.path.abspath(path)
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
    raise ValueError("Couldn't find VW")


def line_diff_text(text_one, file_name_one, text_two, file_name_two):
    text_one = [line.strip() for line in text_one.strip().splitlines()]
    text_two = [line.strip() for line in text_two.strip().splitlines()]

    diff = difflib.unified_diff(
        text_two, text_one, fromfile=file_name_two, tofile=file_name_one, lineterm='')
    output_lines = []
    for line in diff:
        output_lines.append(line)

    return len(output_lines) != 0, output_lines


def is_line_different(output_line, ref_line, epsilon):
    output_tokens = re.split('[ \t:,@]+', output_line)
    ref_tokens = re.split('[ \t:,@]+', ref_line)

    if len(output_tokens) != len(ref_tokens):
        return True, "Number of tokens different", False

    found_close_floats = False
    for output_token, ref_token in zip(output_tokens, ref_tokens):
        output_is_float = is_float(output_token)
        ref_is_float = is_float(ref_token)
        if output_is_float and ref_is_float:
            close = fuzzy_float_compare(output_token, ref_token, epsilon)
            if close:
                # print("close")
                found_close_floats = True
                continue
            else:
                return True, f"Floats don't match {output_token} {ref_token}", found_close_floats
        else:
            if output_token != ref_token:
                return True, f"Mismatch at token {output_token} {ref_token}", found_close_floats

    return False, "", found_close_floats


def are_lines_different(output_lines, ref_lines, epsilon, fuzzy_compare=False):
    if len(output_lines) != len(ref_lines):
        return True, "Diff mismatch"

    found_close_floats = False
    for output_line, ref_line in zip(output_lines, ref_lines):
        if fuzzy_compare:
            # Some output contains '...', remove this for comparison.
            output_line = output_line.replace("...", "")
            ref_line = ref_line.replace("...", "")
            is_different, reason, found_close_floats_temp = is_line_different(
                output_line, ref_line, epsilon)
            found_close_floats = found_close_floats or found_close_floats_temp
            if is_different:
                return True, reason
        else:
            if output_line != ref_line:
                return True, f"Lines differ - ref vs output: '{ref_line}' vs '{output_line}'"

    return False, "Minor float difference ignored" if found_close_floats else ""


def is_diff_different(output_content, output_file_name, ref_content, ref_file_name, epsilon, fuzzy_compare=False):
    is_different, diff = line_diff_text(
        output_content, output_file_name, ref_content, ref_file_name)

    if not is_different:
        return False, [], ""

    output_lines = [line[1:] for line in diff if line.startswith(
        '+') and not line.startswith('+++')]
    ref_lines = [line[1:] for line in diff if line.startswith(
        '-') and not line.startswith('---')]

    # if number of lines different it is a fail
    # if lines are the same, check if number of tokens the same
    # if number of tokens the same, check if they pass float equality

    is_different, reason = are_lines_different(
        output_lines, ref_lines, epsilon, fuzzy_compare=fuzzy_compare)
    diff = diff if is_different else []
    return is_different, diff, reason


def are_outputs_different(output_content, output_file_name, ref_content, ref_file_name, overwrite, epsilon, fuzzy_compare=False):
    is_different, diff, reason = is_diff_different(
        output_content, output_file_name, ref_content, ref_file_name, epsilon, fuzzy_compare=fuzzy_compare)

    if is_different and overwrite:
        with open(ref_file_name, 'w') as writer:
            writer.write(output_content)

    if not is_different:
        return False, [], reason

    # If diff difference fails, fall back to a line by line compare to double check.

    output_lines = [line.strip()
                    for line in output_content.strip().splitlines()]
    ref_lines = [line.strip() for line in ref_content.strip().splitlines()]
    is_different, reason = are_lines_different(
        output_lines, ref_lines, epsilon)
    diff = diff if is_different else []
    return is_different, diff, reason


def is_float(value):
    try:
        float(value)
        return True
    except ValueError:
        return False


def print_colored_diff(diff):
    for line in diff:
        if line.startswith('+'):
            print(Color.OKGREEN + line + Color.ENDC)
        elif line.startswith('-'):
            print(Color.FAIL + line + Color.ENDC)
        elif line.startswith('^'):
            print(line)
        else:
            print(line)


def run_command_line_test(id, command_line, comparison_files, overwrite, epsilon, is_shell, input_files, base_working_dir, ref_dir, dependencies=None, fuzzy_compare=False):
    if dependencies is not None:
        for dep in dependencies:
            success = completed_tests.wait_for_completion_get_success(dep)
            if not success:
                return (id, {
                    "result": Result.SKIPPED,
                    "checks": {}
                })

    try:
        if is_shell:
            working_dir = os.getcwd()
            cmd = command_line
        else:
            working_dir = create_test_dir(
                id, input_files, base_working_dir, ref_dir, dependencies=dependencies)
            cmd = f"{command_line}".split()

        try:
            result = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=working_dir,
                shell=is_shell,
                timeout=10)
        except subprocess.TimeoutExpired as e:
            stdout = try_decode(e.stdout)
            stderr = try_decode(e.stderr)
            checks = dict()
            checks["timeout"] = {
                "success": False,
                "message": f"{e.cmd} timed out",
                "stdout": stdout,
                "stderr": stderr
            }

            return (id, {
                "result": Result.FAIL,
                "checks": checks
            })

        return_code = result.returncode
        stdout = try_decode(result.stdout)
        stderr = try_decode(result.stderr)

        checks = dict()
        checks["error_code"] = {
            "success": return_code == 0,
            "message": f"Exited with {return_code}",
            "stdout": stdout,
            "stderr": stderr
        }

        for output_file, ref_file in comparison_files.items():

            if output_file == "stdout":
                output_content = stdout
            elif output_file == "stderr":
                output_content = stderr
            else:
                output_file_working_dir = os.path.join(
                    working_dir, output_file)
                if os.path.isfile(output_file_working_dir):
                    output_content = open(output_file_working_dir, 'r').read()
                else:
                    checks[output_file] = {
                        "success": False,
                        "message": f"Failed to open output file: {output_file}",
                        "diff": []
                    }
                    continue

            if os.path.isfile(ref_file):
                ref_file_ref_dir = os.path.join(ref_dir, ref_file)
                ref_content = open(ref_file_ref_dir, 'r').read()
            else:
                checks[output_file] = {
                    "success": False,
                    "message": f"Failed to open ref file: {ref_file}",
                    "diff": []
                }
                continue
            are_different, diff, reason = are_outputs_different(output_content, output_file,
                                                                ref_content, ref_file, overwrite, epsilon, fuzzy_compare=fuzzy_compare)

            if are_different:
                message = f"Diff not OK, {reason}"
            else:
                message = f"Diff OK, {reason}"

            checks[output_file] = {
                "success": are_different == False,
                "message": message,
                "diff": diff
            }
    except:
        completed_tests.report_completion(id, False)
        raise

    success = all(check["success"] == True for name, check in checks.items())
    completed_tests.report_completion(id, success)

    return (id, {
        "result": Result.SUCCESS if success else Result.FAIL,
        "checks": checks
    })

class Completion():
    def __init__(self):
        self.lock = threading.Lock()
        self.condition = threading.Condition(self.lock)
        self.completed = dict()

    def report_completion(self, id, success):
        self.lock.acquire()
        self.completed[id] = success
        self.condition.notify_all()
        self.lock.release()

    def wait_for_completion_get_success(self, id):
        def is_complete():
            return id in self.completed
        self.lock.acquire()
        if not is_complete():
            self.condition.wait_for(is_complete)
        success = self.completed[id]
        self.lock.release()
        return success


completed_tests = Completion()


def create_test_dir(id, input_files, test_base_dir, test_ref_dir, dependencies=None):
    test_working_dir = Path(test_base_dir).joinpath(f"test_{id}")
    Path(test_working_dir).mkdir(parents=True, exist_ok=True)

    # Required as workaround until #2686 is fixed.
    Path(test_working_dir.joinpath("models")).mkdir(
        parents=True, exist_ok=True)

    for file in input_files:
        file_to_copy = None
        search_paths = [Path(test_ref_dir).joinpath(file)]
        if dependencies is not None:
            search_paths.extend([Path(test_base_dir).joinpath(
                f"test_{x}", file) for x in dependencies])
        for search_path in search_paths:
            if search_path.exists() and not search_path.is_dir():
                file_to_copy = search_path
                break

        if file_to_copy is None:
            raise ValueError(f"{file} couldn't be found for test {id}")

        test_dest_file = Path(test_working_dir).joinpath(file)
        Path(test_dest_file.parent).mkdir(parents=True, exist_ok=True)
        # We always want to replace this file in case it is the output of another test
        if test_dest_file.exists():
            test_dest_file.unlink()
        shutil.copyfile(file_to_copy, test_dest_file)
    return test_working_dir


def main():

    working_dir = Path.home().joinpath(f".vw_runtests_working_dir")
    test_ref_dir = Path(os.path.dirname(os.path.abspath(__file__)))

    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-t', "--test", type=int,
                        action='extend', nargs='+', help="Run specific tests and ignore all others")
    parser.add_argument('-E', "--epsilon", type=float, default=1e-3, help="Tolerance used when comparing floats. Only used if --fuzzy_compare is also supplied")
    parser.add_argument('-e', "--exit_first_fail", action='store_true', help="If supplied, will exit after the first failure")
    parser.add_argument('-o', "--overwrite", action='store_true', help="If test output differs from the reference file, overwrite the contents")
    parser.add_argument('-f', "--fuzzy_compare", action='store_true', help="Allow for some tolerance when comparing floats")
    parser.add_argument("--ignore_dirty", action='store_true', help="The test ref dir is checked for dirty files which may cause false negatives. Pass this flag to skip this check.")
    parser.add_argument("--working_dir", default=working_dir, help="Directory to save test outputs to")
    parser.add_argument("--ref_dir", default=test_ref_dir, help="Directory to read test input files from")
    parser.add_argument('-j', "--jobs", type=int, default=4, help="Number of tests to run in parallel")
    parser.add_argument('--vw_bin_path', help="Specify VW binary to use. Otherwise, binary will be searched for in build directory")
    parser.add_argument('--spanning_tree_bin_path', help="Specify spanning tree binary to use. Otherwise, binary will be searched for in build directory")
    parser.add_argument("--test_spec", type=str,
                        help="Optional. If passed the given JSON test spec will be used, " + 
                        "otherwise a test spec will be autogenerated from the RunTests test definitions")
    args = parser.parse_args()

    TEST_BASE_WORKING_DIR = args.working_dir
    TEST_BASE_REF_DIR = args.ref_dir

    if Path(TEST_BASE_WORKING_DIR).is_file():
        print(f"--working_dir='{TEST_BASE_WORKING_DIR}' cannot be a file")
        sys.exit(1)
    
    if not Path(TEST_BASE_WORKING_DIR).exists():
        Path(TEST_BASE_WORKING_DIR).mkdir(parents=True, exist_ok=True)

    if not Path(TEST_BASE_REF_DIR):
        print(f"--ref_dir='{TEST_BASE_REF_DIR}' doesn't exist")
        sys.exit(1)

    if not args.ignore_dirty:
        result = subprocess.run(
            "git clean --dry-run -d -x -e __pycache__".split(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=TEST_BASE_REF_DIR,
            timeout=10)
        return_code = result.returncode
        if return_code != 0:
            print("Failed to run 'git clean --dry-run -d -x -e __pycache__'")
        stdout = try_decode(result.stdout)
        if len(stdout) != 0:
            print(f"Error: Test dir is not clean, this can result in false negatives. To ignore this and continue anyway pass --ignore_dirty")
            print(f"'git clean --dry-run -d -x -e __pycache__' output:\n---")
            print(stdout)
            sys.exit(1)
    
    print(f"Testing on: hostname={socket.gethostname()}, OS={sys.platform}")
    
    if args.vw_bin_path is None:
        vw_search_paths = [
            Path(TEST_BASE_REF_DIR).joinpath("../build/vowpalwabbit")
        ]
        def is_vw_binary(file_path):
            file_name = os.path.basename(file_path)
            return file_name == "vw"
        vw_bin = find_in_path(vw_search_paths, is_vw_binary)
    else:
        if not Path(args.vw_bin_path).exists() or not Path(args.vw_bin_path).is_file() :
            print(f"Invalid vw binary path: {args.vw_bin_path}")
        vw_bin = args.vw_bin_path
    
    print(f"Using VW binary: {vw_bin}")

    if args.spanning_tree_bin_path is None:
        spanning_tree_search_path = [
            Path(TEST_BASE_REF_DIR).joinpath("../build/cluster")
        ]
        def is_spanning_tree_binary(file_path):
            file_name = os.path.basename(file_path)
            return file_name == "spanning_tree"
        spanning_tree_bin = find_in_path(spanning_tree_search_path, is_spanning_tree_binary)
    else:
        if not Path(args.spanning_tree_bin_path).exists() or not Path(args.spanning_tree_bin_path).is_file() :
            print(f"Invalid spanning tree binary path: {args.spanning_tree_bin_path}")
        spanning_tree_bin = args.spanning_tree_bin_path

    print(f"Using spanning tree binary: {spanning_tree_bin}")

    
    if args.test_spec is None:
        def is_runtests_file(file_path):
            file_name = os.path.basename(file_path)
            return file_name == "RunTests"
        possible_runtests_paths = [
            Path(TEST_BASE_REF_DIR)
        ]
        runtests_file = find_in_path(possible_runtests_paths, is_runtests_file)
        tests = runtests_parser.file_to_obj(runtests_file)
        tests = [x.__dict__ for x in tests]
        print(f"Tests parsed from RunTests file: {runtests_file}")
    else:
        json_test_spec_content = open(args.test_spec).read()
        tests = json.loads(json_test_spec_content)
        print(f"Tests read from test spec file: {args.test_spec}")

    print()

    tasks = []

    def get_deps(test_number, tests):
        deps = set()
        test_index = test_number - 1 
        if "depends_on" in tests[test_index]:
            for dep in tests[test_index]["depends_on"]:
                deps.add(dep)
                deps = set.union(deps, get_deps(dep, tests))
        return deps

    tests_to_run_explicitly = None
    if args.test is not None:
        tests_to_run_explicitly = set()
        for test_number in args.test:
            tests_to_run_explicitly.add(test_number)
            tests_to_run_explicitly = set.union(tests_to_run_explicitly, get_deps(test_number, tests))
        print(f"Running tests: {list(tests_to_run_explicitly)}")
        if len(args.test) != len(tests_to_run_explicitly):
            print(f"Note: due to test dependencies, more than just tests {args.test} must be run")

    executor = ThreadPoolExecutor(max_workers=args.jobs)
    for test in tests:
        test_number = test["id"]
        if tests_to_run_explicitly is not None and test_number not in tests_to_run_explicitly:
            continue

        dependencies = None
        if "depends_on" in test:
            dependencies = test["depends_on"]

        input_files = []
        if "input_files" in test:
            input_files = test["input_files"]

        is_shell = False
        if "bash_command" in test:
            if sys.platform == "win32":
                print(
                    f"Skipping test number '{test_number}' as bash_command is unsupported on Windows.")
                continue
            command_line = test['bash_command'].format(VW=vw_bin, SPANNING_TREE=spanning_tree_bin)
            is_shell = True
        elif "vw_command" in test:
            command_line = f"{vw_bin} {test['vw_command']}"
        else:
            print(f"{test_number} is an unknown type. Skipping...")
            continue

        tasks.append(executor.submit(run_command_line_test, test_number, command_line, test["diff_files"],
                                     overwrite=args.overwrite, epsilon=args.epsilon, is_shell=is_shell, input_files=input_files, base_working_dir=TEST_BASE_WORKING_DIR, ref_dir=TEST_BASE_REF_DIR, dependencies=dependencies, fuzzy_compare=args.fuzzy_compare))

    num_success = 0
    num_fail = 0
    num_skip = 0
    while len(tasks) > 0:
        try:
            test_number, result = tasks[0].result()
        except Exception:
            print("----------------")
            traceback.print_exc()
            num_fail += 1
            print("----------------")
            if args.exit_first_fail:
                for task in tasks:
                    task.cancel()
                sys.exit(1)
            continue
        finally:
            tasks.pop(0)

        success_text = f"{Color.OKGREEN}Success{Color.ENDC}"
        fail_text = f"{Color.FAIL}Fail{Color.ENDC}"
        skipped_text = f"{Color.OKCYAN}Skip{Color.ENDC}"
        num_success += result['result'] == Result.SUCCESS
        num_fail += result['result'] == Result.FAIL
        num_skip += result['result'] == Result.SKIPPED

        if result['result'] == Result.SUCCESS:
            result_text = success_text
        elif result['result'] == Result.FAIL:
            result_text = fail_text
        else:
            result_text = skipped_text

        print(f"Test {test_number}: {result_text}")
        if not result['result'] == Result.SUCCESS:
            test = tests[test_number - 1]
            print(f"\tDescription: {test['desc']}")
            if 'vw_command' in test:
                print(f"\tvw_command: \"{test['vw_command']}\"")
            if 'bash_command' in test:
                print(f"\tbash_command: \"{test['bash_command']}\"")
        for name, check in result["checks"].items():
            print(
                f"\t[{name}] {success_text if check['success'] else fail_text}: {check['message']}")
            if not check['success']:
                if name == "error_code":
                    print("---- stdout ----")
                    print(result["checks"]["error_code"]["stdout"])
                    print("---- stderr ----")
                    print(result["checks"]["error_code"]["stderr"])

                if "diff" in check:
                    print()
                    if(len(check["diff"]) > 100):
                        print("Skipping large diff for now...")
                    else:
                        print_colored_diff(check["diff"])
                    print()
                if args.exit_first_fail:
                    for task in tasks:
                        task.cancel()
                    sys.exit(1)
    print(f"-----")
    print(f"# Success: {num_success}")
    print(f"# Fail: {num_fail}")
    print(f"# Skip: {num_skip}")


if __name__ == "__main__":
    main()
