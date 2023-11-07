import shutil
import threading
import argparse
import difflib
from pathlib import Path
import re
import os
import subprocess
import sys
import traceback
import shlex
import math
import json
from concurrent.futures import ThreadPoolExecutor, Future
from enum import Enum
import socket
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

from run_tests_common import TestData
import runtests_flatbuffer_converter as fb_converter


class Color:
    LIGHT_CYAN = "\033[96m"
    LIGHT_GREEN = "\033[92m"
    LIGHT_PURPLE = "\033[95m"
    LIGHT_RED = "\033[91m"
    ENDC = "\033[0m"


class NoColor:
    LIGHT_CYAN = ""
    LIGHT_GREEN = ""
    LIGHT_PURPLE = ""
    LIGHT_RED = ""
    ENDC = ""


class Result(Enum):
    SUCCESS = 1
    FAIL = 2
    SKIPPED = 3


def are_dicts_equal(
    expected_dict: Mapping[Any, Any], actual_dict: Mapping[Any, Any], epsilon: float
) -> Tuple[bool, str]:
    def _are_same(expected: Any, actual: Any, key: str) -> Tuple[bool, str]:
        if type(expected) != type(actual):
            return (
                False,
                f"Key '{key}' type mismatch. Expected: '{type(expected)}', but found: '{type(actual)}'",
            )
        elif type(expected) == type(None):
            return True, ""
        elif isinstance(expected, (int, bool, str)):
            return (
                expected == actual,
                f"Key '{key}' value mismatch. Expected: '{expected}', but found: '{actual}'"
                if expected != actual
                else "",
            )
        elif isinstance(expected, (float)):
            delta = abs(expected - actual)
            return (
                delta < epsilon,
                f"Key '{key}' value mismatch. Expected: '{expected}', but found: '{actual}' (using epsilon: '{epsilon}')"
                if delta >= epsilon
                else "",
            )
        elif isinstance(expected, dict):
            expected_keys = set(expected.keys())
            actual_keys = set(actual.keys())
            if expected_keys != actual_keys:
                return (
                    False,
                    f"Key '{key}' keys mismatch. Expected: {expected_keys.difference(actual_keys)}. Found but not expected: {actual_keys.difference(expected_keys)}",
                )
            for key in expected_keys:
                are_same, reason = _are_same(expected[key], actual[key], f"{key}")
                if not are_same:
                    return False, reason
            return True, ""
        elif isinstance(expected, list):
            if len(expected) != len(actual):
                return (
                    False,
                    f"Key '{key}' length mismatch. Expected: '{len(expected)}', but found: '{len(actual)}'",
                )
            for i in range(len(expected)):
                are_same, reason = _are_same(expected[i], actual[i], f"{key}[{i}]")
                if not are_same:
                    return False, reason
            return True, ""
        else:
            raise TypeError(f"Type {type(expected)} not supported in are_dicts_equal")

    return _are_same(expected_dict, actual_dict, "root")


class Completion:
    def __init__(self):
        self.lock = threading.Lock()
        self.condition = threading.Condition(self.lock)
        self.completed: Dict[int, bool] = dict()

    def report_completion(self, id: int, success: bool) -> None:
        self.lock.acquire()
        self.completed[id] = success
        self.condition.notify_all()
        self.lock.release()

    def wait_for_completion_get_success(self, id: int) -> bool:
        def is_complete() -> bool:
            return id in self.completed

        self.lock.acquire()
        if not is_complete():
            self.condition.wait_for(is_complete)
        success = self.completed[id]
        self.lock.release()
        return success


class StatusCheck:
    success: bool
    message: str
    stdout: str
    stderr: str

    def __init__(self, success: bool, message: str, stdout: str, stderr: str):
        self.success = success
        self.message = message
        self.stdout = stdout
        self.stderr = stderr


class DiffCheck:
    success: bool
    message: str
    diff: List[str]

    def __init__(self, success: bool, message: str, diff: List[str]):
        self.success = success
        self.message = message
        self.diff = diff


class TestOutcome:
    id: int
    result: Result
    checks: Dict[str, Union[StatusCheck, DiffCheck]]
    skip_reason: Optional[str]

    def __init__(
        self,
        id: int,
        result: Result,
        checks: Dict[str, Union[StatusCheck, DiffCheck]],
        skip_reason: Optional[str] = None,
    ):
        self.id = id
        self.result = result
        self.checks = checks
        self.skip_reason = skip_reason


def try_decode(binary_object: Optional[bytes]) -> str:
    # ignore UTF decode errors so that we can still understand the output in this case
    return binary_object.decode("utf-8", "ignore") if binary_object is not None else ""


# Returns true if they are close enough to be considered equal.
def fuzzy_float_compare(float_one_str: str, float_two_str: str, epsilon: float) -> bool:
    float_one = float(float_one_str)
    float_two = float(float_two_str)

    # Special case handle these two as they will not be equal when checking absolute difference.
    # But for the purposes of comparing the diff they are equal.
    if math.isinf(float_one) and math.isinf(float_two):
        return True
    if math.isnan(float_one) and math.isnan(float_two):
        return True

    delta = abs(float_one - float_two)
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
    if abs(float_two) <= 1.0:
        return False

    # Now we can safely divide (since abs($word2) > 0) and determine the ratio difference from 1.0
    ratio_delta = abs(float_one / float_two - 1.0)
    return ratio_delta < epsilon


def find_in_path(
    paths: List[Path], file_matcher: Callable[[Path], bool]
) -> Optional[Path]:
    for path in paths:
        if path.is_dir():
            for file in path.iterdir():
                if file_matcher(file):
                    return file
        elif path.is_file():
            if file_matcher(path):
                return path
        else:
            # path does not exist
            continue
    return None


def create_file_diff(
    text_one: str, file_name_one: str, text_two: str, file_name_two: str
) -> List[str]:
    text_one_list = [line.strip() for line in text_one.strip().splitlines()]
    text_two_list = [line.strip() for line in text_two.strip().splitlines()]
    diff = difflib.unified_diff(
        text_two_list,
        text_one_list,
        fromfile=file_name_two,
        tofile=file_name_one,
        lineterm="",
    )
    return list([line for line in diff])


def is_line_different(
    output_line: str, ref_line: str, epsilon: float
) -> Tuple[bool, str, bool]:
    output_tokens = re.split("[ \t:,@=]+", output_line)
    ref_tokens = re.split("[ \t:,@=]+", ref_line)

    # some compile flags cause VW to report different code line number for the same exception
    # if this is the case we want to ignore that from the diff
    if ref_tokens[0] == "[critical]" and output_tokens[0] == "[critical]":
        # check that exception format is being followed
        if ref_tokens[2][0] == "(" and ref_tokens[3][-1] == ")":
            if ref_tokens[3][:-1].isnumeric():
                # remove the line number before diffing
                ref_tokens.pop(3)
                output_tokens.pop(3)

    if len(output_tokens) != len(ref_tokens):
        return True, "Number of tokens different", False

    found_close_floats = False
    for output_token, ref_token in zip(output_tokens, ref_tokens):
        output_is_float = is_float(output_token)
        ref_is_float = is_float(ref_token)
        if output_is_float and ref_is_float:
            close = fuzzy_float_compare(output_token, ref_token, epsilon)
            if close:
                found_close_floats = True
                continue

            return (
                True,
                f"Floats don't match {output_token} {ref_token}",
                found_close_floats,
            )
        else:
            if output_token != ref_token:
                return (
                    True,
                    f"Mismatch at token {output_token} {ref_token}",
                    found_close_floats,
                )

    # ignore whitespace when considering delimiting tokens
    output_delimiters = re.findall("[:,@]+", output_line)
    ref_delimiters = re.findall("[:,@]+", ref_line)

    if len(output_delimiters) != len(ref_delimiters):
        return True, "Number of tokens different", found_close_floats

    for output_token, ref_token in zip(output_delimiters, ref_delimiters):
        if output_token != ref_token:
            return (
                True,
                f"Mismatch at token {output_token} {ref_token}",
                found_close_floats,
            )

    return False, "", found_close_floats


def are_lines_different(
    output_lines: List[str], ref_lines: List[str], epsilon: float, fuzzy_compare=False
) -> Tuple[bool, str]:
    if len(output_lines) != len(ref_lines):
        return True, "Diff mismatch"

    found_close_floats = False
    for output_line, ref_line in zip(output_lines, ref_lines):
        if fuzzy_compare:
            # Some output contains '...', remove this for comparison.
            output_line = output_line.replace("...", "")
            ref_line = ref_line.replace("...", "")
            is_different, reason, found_close_floats_temp = is_line_different(
                output_line, ref_line, epsilon
            )
            found_close_floats = found_close_floats or found_close_floats_temp
            if is_different:
                return True, reason
        else:
            if output_line != ref_line:
                return (
                    True,
                    f"Lines differ - ref vs output: '{ref_line}' vs '{output_line}'",
                )

    return False, "Minor float difference ignored" if found_close_floats else ""


def is_output_different(
    output_content: str,
    ref_content: str,
    epsilon: float,
    fuzzy_compare=False,
) -> Tuple[bool, str]:
    output_lines = [line.strip() for line in output_content.strip().splitlines()]
    ref_lines = [line.strip() for line in ref_content.strip().splitlines()]
    is_different, reason = are_lines_different(
        output_lines, ref_lines, epsilon, fuzzy_compare=fuzzy_compare
    )
    return is_different, reason


def is_float(value: str) -> bool:
    try:
        float(value)
        return True
    except ValueError:
        return False


def print_colored_diff(diff, color_enum: Type[Union[Color, NoColor]]):
    for line in diff:
        if line.startswith("+"):
            print(color_enum.LIGHT_GREEN + line + color_enum.ENDC)
        elif line.startswith("-"):
            print(color_enum.LIGHT_RED + line + color_enum.ENDC)
        elif line.startswith("^"):
            print(line)
        else:
            print(line)


def is_valgrind_available() -> bool:
    return shutil.which("valgrind") is not None


def run_command_line_test(
    test: TestData,
    overwrite: bool,
    epsilon: bool,
    base_working_dir: Path,
    ref_dir: Path,
    completed_tests: Completion,
    fuzzy_compare=False,
    valgrind=False,
    timeout=100,
) -> TestOutcome:
    if test.skip:
        completed_tests.report_completion(test.id, False)
        return TestOutcome(test.id, Result.SKIPPED, {}, skip_reason=test.skip_reason)

    for dep in test.depends_on:
        success = completed_tests.wait_for_completion_get_success(dep)
        if not success:
            completed_tests.report_completion(test.id, False)
            return TestOutcome(
                test.id,
                Result.SKIPPED,
                {},
                skip_reason=f"Test {test.id} depends on test {dep} which failed.",
            )

    try:
        current_test_working_dir = create_test_dir(
            test.id,
            test.input_files,
            base_working_dir,
            ref_dir,
            dependencies=test.depends_on,
        )

        command_line = test.command_line
        if valgrind:
            valgrind_log_file_path = (
                current_test_working_dir / f"test-{test.id}.valgrind-err"
            )
            command_line = f"valgrind --quiet --error-exitcode=100 --track-origins=yes --leak-check=full --log-file={valgrind_log_file_path} {command_line}"

        cmd: Union[str, List[str]]
        if test.is_shell:
            cmd = command_line
        else:
            posix = sys.platform != "win32"
            cmd = shlex.split(command_line, posix=posix)

        checks: Dict[str, Union[StatusCheck, DiffCheck]] = dict()
        try:
            result = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=current_test_working_dir,
                shell=test.is_shell,
                timeout=timeout,
            )
        except subprocess.TimeoutExpired as e:
            stdout = try_decode(e.stdout)
            stderr = try_decode(e.stderr)

            checks["timeout"] = StatusCheck(False, f"{e.cmd} timed out", stdout, stderr)
            return TestOutcome(
                test.id, Result.FAIL, checks, skip_reason=test.skip_reason
            )

        return_code = result.returncode
        stdout = try_decode(result.stdout)
        stderr = try_decode(result.stderr)
        success = return_code == 0 or (
            return_code == 100 and test.is_shell and valgrind
        )
        message = f"Exited with {return_code}"
        if return_code == 100 and test.is_shell and valgrind:
            message += " - valgrind failure ignored in shell test"
        checks["exit_code"] = StatusCheck(success, message, stdout, stderr)

        if valgrind:
            success = True
            message = "OK"
            diff = []
            if return_code == 100:
                if test.is_shell:
                    message = "valgrind failure ignored for a shell based test"
                else:
                    success = False
                    message = f"valgrind failed with command: '{command_line}'"
                    diff = (
                        valgrind_log_file_path.open("r", encoding="utf-8")
                        .read()
                        .split("\n")
                    )
            elif return_code != 0:
                success = False
                message = "non-valgrind failure error code"

            checks["valgrind"] = DiffCheck(success, message, diff)

        for output_file, ref_file in test.comparison_files.items():
            if output_file == "stdout":
                output_content = stdout
            elif output_file == "stderr":
                output_content = stderr
            else:
                output_file_working_dir = current_test_working_dir / output_file
                if output_file_working_dir.is_file():
                    output_content = output_file_working_dir.open(
                        "r", encoding="utf-8"
                    ).read()
                else:
                    checks[output_file] = DiffCheck(
                        False, f"Failed to open output file: {output_file}", []
                    )
                    continue

            ref_file_ref_dir = ref_dir / ref_file
            if ref_file_ref_dir.is_file():
                ref_content = ref_file_ref_dir.open("r", encoding="utf-8").read()
            else:
                checks[output_file] = DiffCheck(
                    False, f"Failed to open ref file: {ref_file}", []
                )
                continue

            if ref_file.endswith(".json"):
                # Empty strings are falsy
                output_json = json.loads(output_content) if output_content else dict()
                ref_json = json.loads(ref_content) if ref_content else dict()
                dicts_equal, reason = are_dicts_equal(
                    output_json, ref_json, epsilon=epsilon
                )
                is_different = not dicts_equal
            else:
                is_different, reason = is_output_different(
                    output_content, ref_content, epsilon, fuzzy_compare=fuzzy_compare
                )

            if is_different and overwrite:
                with ref_file_ref_dir.open("w") as writer:
                    writer.write(output_content)

            diff = create_file_diff(output_content, output_file, ref_content, ref_file)
            if is_different:
                message = f"Diff not OK, {reason}"
            else:
                message = f"Diff OK, {reason}"
            checks[output_file] = DiffCheck(is_different is False, message, diff)
    except:
        completed_tests.report_completion(test.id, False)
        raise

    success = all(check.success for _, check in checks.items())
    completed_tests.report_completion(test.id, success)

    return TestOutcome(test.id, Result.SUCCESS if success else Result.FAIL, checks)


def create_test_dir(
    test_id: int,
    input_files: List[str],
    test_base_dir: Path,
    test_ref_dir: Path,
    dependencies: List[int] = [],
) -> Path:
    test_working_dir = test_base_dir.joinpath(f"test_{test_id}")
    test_working_dir.mkdir(parents=True, exist_ok=True)

    # Required as workaround until #2686 is fixed.
    (test_working_dir / "models").mkdir(parents=True, exist_ok=True)

    for f in input_files:
        file_to_copy = None
        search_paths = [test_ref_dir / f]
        if len(dependencies) > 0:
            search_paths.extend(
                [(test_base_dir / f"test_{x}" / f) for x in dependencies]
            )
            search_paths.extend(
                [
                    (test_base_dir / f"test_{x}" / os.path.basename(f))
                    for x in dependencies
                ]
            )  # for input_files with a full path
        for search_path in search_paths:
            if search_path.exists() and not search_path.is_dir():
                file_to_copy = search_path
                break

        if file_to_copy is None:
            str_deps = [str(dep) for dep in dependencies]
            dependent_tests = ", ".join(str_deps)
            raise ValueError(
                f"Input file '{f}' couldn't be found for test {test_id}. Searched in '{test_ref_dir}' as well as outputs of dependent tests: [{dependent_tests}]"
            )

        test_dest_file = test_working_dir / f
        if file_to_copy == test_dest_file:
            continue
        test_dest_file.parent.mkdir(parents=True, exist_ok=True)
        # We always want to replace this file in case it is the output of another test
        if test_dest_file.exists():
            test_dest_file.unlink()
        shutil.copy(str(file_to_copy), str(test_dest_file))
    return test_working_dir


def find_vw_binary(
    test_base_ref_dir: Path, user_supplied_bin_path_or_python_invocation: Optional[str]
) -> Optional[Union[str, Path]]:
    def is_python_invocation(binary_name: Optional[str]) -> bool:
        if not binary_name:
            return False
        elif binary_name.startswith("python") and binary_name.endswith(
            "-m vowpalwabbit"
        ):
            return True
        else:
            return False

    if is_python_invocation(user_supplied_bin_path_or_python_invocation):
        return user_supplied_bin_path_or_python_invocation

    user_supplied_bin_path = (
        Path(user_supplied_bin_path_or_python_invocation)
        if user_supplied_bin_path_or_python_invocation is not None
        else None
    )
    vw_search_paths = [test_base_ref_dir / ".." / "build" / "vowpalwabbit" / "cli"]

    def is_vw_binary(file: Path) -> bool:
        return file.name == "vw" or file.name == "vw.exe"

    return find_or_use_user_supplied_path(
        test_base_ref_dir=test_base_ref_dir,
        user_supplied_bin_path=user_supplied_bin_path,
        search_paths=vw_search_paths,
        is_correct_bin_func=is_vw_binary,
    )


def find_spanning_tree_binary(
    test_base_ref_dir: Path, user_supplied_bin_path: Optional[str]
) -> Optional[Path]:
    spanning_tree_search_path = [
        test_base_ref_dir / ".." / "build" / "vowpalwabbit" / "spanning_tree_bin"
    ]

    def is_spanning_tree_binary(file: Path) -> bool:
        return file.name == "spanning_tree" or file.name == "spanning_tree.exe"

    user_supplied_bin_path = (
        Path(user_supplied_bin_path) if user_supplied_bin_path is not None else None
    )

    return find_or_use_user_supplied_path(
        test_base_ref_dir=test_base_ref_dir,
        user_supplied_bin_path=user_supplied_bin_path,
        search_paths=spanning_tree_search_path,
        is_correct_bin_func=is_spanning_tree_binary,
    )


def find_to_flatbuf_binary(
    test_base_ref_dir: Path, user_supplied_bin_path: Optional[str]
) -> Optional[Path]:
    to_flatbuff_search_path = [
        test_base_ref_dir / ".." / "build" / "utl" / "flatbuffer"
    ]

    def is_to_flatbuff_binary(file: Path) -> bool:
        return file.name == "to_flatbuff" or file.name == "to_flatbuff.exe"

    user_supplied_bin_path = (
        Path(user_supplied_bin_path) if user_supplied_bin_path is not None else None
    )

    return find_or_use_user_supplied_path(
        test_base_ref_dir=test_base_ref_dir,
        user_supplied_bin_path=user_supplied_bin_path,
        search_paths=to_flatbuff_search_path,
        is_correct_bin_func=is_to_flatbuff_binary,
    )


def find_or_use_user_supplied_path(
    test_base_ref_dir: Path,
    user_supplied_bin_path: Optional[Path],
    search_paths: List[Path],
    is_correct_bin_func: Callable[[Path], bool],
) -> Optional[Path]:
    if user_supplied_bin_path is None:
        return find_in_path(search_paths, is_correct_bin_func)
    if not user_supplied_bin_path.is_file():
        return None
    return user_supplied_bin_path


def do_dirty_check(test_base_ref_dir: Path) -> None:
    result = subprocess.run(
        "git clean --dry-run -d -x -e __pycache__".split(),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=test_base_ref_dir,
        timeout=10,
    )
    return_code = result.returncode
    if return_code != 0:
        print("Failed to run 'git clean --dry-run -d -x -e __pycache__'")
    stdout = try_decode(result.stdout)
    if len(stdout) != 0:
        print(
            "Error: Test dir is not clean, this can result in false negatives. To ignore this and continue anyway pass --ignore_dirty or pass --clean_dirty to clean"
        )
        print("'git clean --dry-run -d -x -e __pycache__' output:\n---")
        print(stdout)
        sys.exit(1)


def clean_dirty(test_base_ref_dir: Path) -> None:
    git_command = "git clean --force -d -x --exclude __pycache__"
    result = subprocess.run(
        git_command.split(),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=test_base_ref_dir,
        timeout=10,
    )

    if result.returncode != 0:
        print(f"Failed to run {git_command}")


def calculate_test_to_run_explicitly(
    explicit_tests: List[int], tests: List[TestData]
) -> List[int]:
    def get_deps(test_number: int, tests: List[TestData]):
        deps: Set[int] = set()
        test_index = test_number - 1
        for dep in tests[test_index].depends_on:
            deps.add(dep)
            deps = set.union(deps, get_deps(dep, tests))
        return deps

    tests_to_run_explicitly: Set[int] = set()
    for test_number in explicit_tests:
        if test_number > len(tests):
            raise ValueError(
                f"Error: Test number {test_number} does not exist. There are {len(tests)} tests in total."
            )
        tests_to_run_explicitly.add(test_number)
        tests_to_run_explicitly = set.union(
            tests_to_run_explicitly, get_deps(test_number, tests)
        )

    return list(tests_to_run_explicitly)


def convert_tests_for_flatbuffers(
    tests: List[TestData],
    to_flatbuff: Path,
    working_dir: Path,
    color_enum: Type[Union[Color, NoColor]],
):
    working_dir.mkdir(parents=True, exist_ok=True)
    for test in tests:
        if test.is_shell:
            test.skip = True
            test.skip_reason = "Cannot convert bash based tests to flatbuffers"
            continue
        if "flatbuffer" in test.command_line:
            test.skip = True
            test.skip_reason = "already a flatbuffer test"
            continue
        if "malformed" in test.command_line:
            test.skip = True
            test.skip_reason = "malformed input"
            continue
        if len(test.input_files) < 1:
            test.skip = True
            test.skip_reason = (
                "no input files for for automatic converted flatbuffer test"
            )
            continue
        if "dictionary" in test.command_line:
            test.skip = True
            test.skip_reason = "currently dictionaries are not supported for automatic converted flatbuffer tests"
            continue
        if "help" in test.command_line:
            test.skip = True
            test.skip_reason = (
                "--help test skipped for automatic converted flatbuffer tests"
            )
            continue
        # todo: 300 understand why is it failing
        # test 189, 312, 316, 318, 319, 351, 367, 368, 394, 438, 456, 457 depend on dsjson parser behaviour
        # they can be enabled if we ignore diffing the --extra_metrics
        # (324-326) deals with corrupted data, so cannot be translated to fb
        # pdrop is not supported in fb, so 327-331 are excluded
        # 336, 337, 338, 442, 444, 450, 452 - the FB converter script seems to be affecting the invert_hash
        # 423, 424, 425, 426 - FB converter removes feature names from invert_hash (probably the same issue as above)
        if str(test.id) in (
            "300",
            "189",
            "312",
            "316",
            "318",
            "319",
            "324",
            "325",
            "326",
            "327",
            "328",
            "329",
            "330",
            "331",
            "336",
            "337",
            "338",
            "351",
            "367",
            "368",
            "394",
            "399",
            "400",
            "404",
            "405",
            "406",
            "407",
            "411",
            "415",
            "426",
            "428",
            "438",
            "442",
            "444",
            "450",
            "452",
            "456",
            "457",
            "458",
            "459",
            "460",
            "461",
            "462",
        ):
            test.skip = True
            test.skip_reason = "test skipped for automatic converted flatbuffer tests for unknown reason"
            continue

        # test id is being used as an index here, not necessarily a contract
        depends_on_test = (
            get_test(test.depends_on[0], tests) if len(test.depends_on) > 0 else None
        )

        fb_test_converter = fb_converter.FlatbufferTest(
            test, working_dir, depends_on_test=depends_on_test
        )
        fb_test_converter.to_flatbuffer(to_flatbuff, color_enum)

    return tests


def check_test_ids(tests: List[Any]) -> None:
    seen_ids: Set[int] = set()
    for test in tests:
        if "id" not in test:
            raise ValueError(f"id field missing in test: {test}")
        if test["id"] in seen_ids:
            raise ValueError(f"Duplicate found for id: {test['id']}")
        seen_ids.add(test["id"])

    first_id = min(seen_ids)
    if first_id != 1:
        raise ValueError(f"Ids must start from 1. First id was: {first_id}")

    last_test_id = max(seen_ids)
    if len(seen_ids) != (last_test_id):
        missing_ids = []
        for i in range(1, last_test_id + 1):
            if i not in seen_ids:
                missing_ids.append(i)
        raise ValueError(
            f"Missing test ids: [{', '.join(str(x) for x in missing_ids)}]"
        )


def convert_to_test_data(
    tests: List[Any],
    vw_bin: str,
    spanning_tree_bin: Optional[Path],
    skipped_ids: List[int],
    skip_network_tests: bool,
    extra_vw_options: str,
) -> List[TestData]:
    results: List[TestData] = []
    for test in tests:
        skip = False
        skip_reason = None
        is_shell = False
        command_line = ""
        if "bash_command" in test:
            command_line = test["bash_command"].format(
                VW=vw_bin, SPANNING_TREE=spanning_tree_bin
            )
            is_shell = True

            if sys.platform == "darwin" and (
                "daemon" in test["bash_command"]
                or "spanning_tree" in test["bash_command"]
                or "sender_test.py" in test["bash_command"]
                or "active_test.py" in test["bash_command"]
            ):
                skip = True
                skip_reason = "daemon not currently supported in MacOS"
            elif sys.platform == "win32":
                skip = True
                skip_reason = "bash_command is unsupported on Windows"

            if spanning_tree_bin is None and (
                "SPANNING_TREE" in test["bash_command"]
                or "spanning_tree" in test["bash_command"]
            ):
                skip = True
                skip_reason = "Test using spanning_tree skipped because of --skip_spanning_tree_tests argument"

            if skip_network_tests and (
                "daemon" in test["bash_command"]
                or "spanning_tree" in test["bash_command"]
                or "sender_test.py" in test["bash_command"]
                or "active_test.py" in test["bash_command"]
            ):
                skip = True
                skip_reason = "Tests requiring daemon or network skipped because of --skip_network_tests argument"
        elif "vw_command" in test:
            command_line = f"{vw_bin} {test['vw_command']} {extra_vw_options}"
        else:
            skip = True
            skip_reason = "This test is an unknown type"

        if test["id"] in skipped_ids:
            skip = True
            skip_reason = "Test skipped by --skip_test argument"

        results.append(
            TestData(
                id=test["id"],
                description=test["desc"] if "desc" in test else "",
                depends_on=test["depends_on"] if "depends_on" in test else [],
                command_line=command_line,
                is_shell=is_shell,
                input_files=test["input_files"] if "input_files" in test else [],
                comparison_files=test["diff_files"] if "diff_files" in test else dict(),
                skip=skip,
                skip_reason=skip_reason,
            )
        )
    return results


def get_test(test_number: int, tests: List[TestData]) -> Optional[TestData]:
    for test in tests:
        if test.id == test_number:
            return test
    return None


def interpret_test_arg(arg: str, *, num_tests: int) -> List[int]:
    single_number_pattern = re.compile(r"^\d+$")
    range_pattern = re.compile(r"^(\d+)?\.\.(\d+)?$")
    range_pattern_match = range_pattern.match(arg)
    if single_number_pattern.match(arg):
        return [int(arg)]
    elif range_pattern_match:
        start, end = range_pattern_match.groups()
        start = int(start) if start else 1
        end = int(end) if end else num_tests
        if start > end:
            raise ValueError(f"Invalid range: {arg}")
        return list(range(start, end + 1))
    else:
        raise ValueError(
            f"Invalid test argument '{arg}'. Must either be a single integer 'id' or a range in the form 'start..end'"
        )


def main():
    working_dir: Path = Path.home() / ".vw_runtests_working_dir"
    test_ref_dir: Path = Path(__file__).resolve().parent

    default_test_spec_file: Path = test_ref_dir / "core.vwtest.json"

    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=1,
        help="Number of tests to run in parallel. Set to 1 to run tests sequentially.",
    )
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "-t",
        "--test",
        type=str,
        action="append",
        nargs="+",
        help="Run specific tests and ignore all others. Can specific as a single integer 'id' or a range in the form 'start..end'",
    )
    parser.add_argument(
        "-E",
        "--epsilon",
        type=float,
        default=1e-4,
        help="Tolerance used when comparing floats. Only used if --fuzzy_compare is also supplied",
    )
    parser.add_argument(
        "-e",
        "--exit_first_fail",
        action="store_true",
        help="If supplied, will exit after the first failure",
    )
    parser.add_argument(
        "-o",
        "--overwrite",
        action="store_true",
        help="If test output differs from the reference file, overwrite the contents",
    )
    parser.add_argument(
        "-f",
        "--fuzzy_compare",
        action="store_true",
        help="Allow for some tolerance when comparing floats",
    )
    parser.add_argument(
        "--ignore_dirty",
        action="store_true",
        help="The test ref dir is checked for dirty files which may cause false negatives. Pass this flag to skip this check.",
    )
    parser.add_argument(
        "--clean_dirty",
        action="store_true",
        help="The test ref dir is checked for dirty files which may cause false negatives. Pass this flag to remove those files.",
    )
    parser.add_argument(
        "--working_dir",
        default=str(working_dir),
        help="Directory to save test outputs to",
    )
    parser.add_argument(
        "--ref_dir",
        default=str(test_ref_dir),
        help="Directory to read test input files from",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=os.cpu_count(),
        help="Number of tests to run in parallel. Default is current machine core count.",
    )
    parser.add_argument(
        "--vw_bin_path",
        help="Specify VW binary to use. Otherwise, binary will be searched for in build directory",
    )
    parser.add_argument(
        "--spanning_tree_bin_path",
        help="Specify spanning tree binary to use. Otherwise, binary will be searched for in build directory",
    )
    parser.add_argument(
        "--skip_spanning_tree_tests",
        help="Skip tests that use spanning tree",
        action="store_true",
    )
    parser.add_argument(
        "--skip_network_tests",
        help="Skip all tests that require daemon or network connection",
        action="store_true",
    )
    parser.add_argument(
        "--skip_test",
        help="Skip specific test ids",
        nargs="+",
        default=[],
        type=int,
    )
    parser.add_argument(
        "--test_spec",
        type=str,
        default=str(default_test_spec_file),
        help="Which vwtest test specification file to run",
    )
    parser.add_argument(
        "--no_color", action="store_true", help="Don't print color ANSI escape codes"
    )
    parser.add_argument(
        "--for_flatbuffers",
        action="store_true",
        help="Transform all of the test inputs into flatbuffer format and run tests",
    )
    parser.add_argument(
        "--to_flatbuff_path",
        help="Specify to_flatbuff binary to use. Otherwise, binary will be searched for in build directory",
    )
    parser.add_argument(
        "--include_flatbuffers",
        action="store_true",
        help="Don't skip the explicit flatbuffer tests from default run_tests run",
    )
    parser.add_argument(
        "--valgrind", action="store_true", help="Run tests with Valgrind"
    )
    parser.add_argument(
        "-O",
        "--extra_options",
        type=str,
        help="Append extra options to VW command line tests.",
        default="",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        help="How long a single test can run for in seconds.",
        default=100,
    )
    args = parser.parse_args()

    if args.skip_network_tests:
        args.skip_spanning_tree_tests = True

    # user did not supply dir
    temp_working_dir: Path = Path(args.working_dir)
    if args.for_flatbuffers and args.working_dir == str(working_dir):
        temp_working_dir = Path.home() / ".vw_fb_runtests_working_dir"

    test_base_working_dir: Path = temp_working_dir
    test_base_ref_dir: Path = Path(args.ref_dir)

    color_enum = NoColor if args.no_color else Color

    if args.valgrind and not is_valgrind_available():
        print("Can't find valgrind")
        sys.exit(1)

    if test_base_working_dir.is_file():
        print(f"--working_dir='{test_base_working_dir}' cannot be a file")
        sys.exit(1)
    test_base_working_dir.mkdir(parents=True, exist_ok=True)

    if not test_base_ref_dir.is_dir():
        print(f"--ref_dir='{test_base_ref_dir}' doesn't exist")
        sys.exit(1)

    if args.clean_dirty:
        clean_dirty(test_base_ref_dir)

    if not args.ignore_dirty:
        do_dirty_check(test_base_ref_dir)

    print(
        f"Testing on: hostname={socket.gethostname()}, OS={sys.platform}, num_jobs={args.jobs}"
    )

    vw_bin = find_vw_binary(test_base_ref_dir, args.vw_bin_path)
    if vw_bin is None:
        print("Can't find vw binary. Did you build the 'vw_cli_bin' target?")
        sys.exit(1)
    # test if vw_bin is a Path object
    elif isinstance(vw_bin, Path):
        vw_bin = str(vw_bin.resolve())
    print(f"Using VW binary: {vw_bin}")

    spanning_tree_bin: Optional[Path] = None
    if not args.skip_spanning_tree_tests:
        spanning_tree_bin = find_spanning_tree_binary(
            test_base_ref_dir, args.spanning_tree_bin_path
        )
        if spanning_tree_bin is None:
            print(
                "Can't find spanning tree binary. Did you build the 'spanning_tree' target?"
            )
            sys.exit(1)
        else:
            spanning_tree_bin = spanning_tree_bin.resolve()

        print(f"Using spanning tree binary: {spanning_tree_bin.resolve()}")

    test_spec_path = Path(args.test_spec)
    if not test_spec_path.is_file():
        print(f"--test_spec='{test_spec_path}' doesn't exist")
        sys.exit(1)
    json_test_spec_content = open(test_spec_path).read()
    tests = json.loads(json_test_spec_content)
    print(f"Tests read from file: {test_spec_path.resolve()}")

    check_test_ids(tests)

    tests = convert_to_test_data(
        tests,
        vw_bin,
        spanning_tree_bin,
        args.skip_test,
        args.skip_network_tests,
        extra_vw_options=args.extra_options,
    )

    # Flatten nested lists for arg.test argument and process
    # Ideally we would have used action="extend", but that was added in 3.8
    interpreted_test_arg: Optional[List[int]] = None
    if args.test is not None:
        interpreted_test_arg = []
        for arg in args.test:
            for value in arg:
                interpreted_test_arg.extend(
                    interpret_test_arg(value, num_tests=len(tests))
                )

    print()

    # Filter the test list if the requested tests were explicitly specified
    tests_to_run_explicitly = None
    if interpreted_test_arg is not None:
        tests_to_run_explicitly = calculate_test_to_run_explicitly(
            interpreted_test_arg, tests
        )
        print(f"Running tests: {list(tests_to_run_explicitly)}")
        if len(interpreted_test_arg) != len(tests_to_run_explicitly):
            print(
                f"Note: due to test dependencies, more than just tests {interpreted_test_arg} must be run"
            )
        tests = list(filter(lambda x: x.id in tests_to_run_explicitly, tests))

    # Filter out flatbuffer tests if not specified
    if not args.include_flatbuffers and not args.for_flatbuffers:
        for test in tests:
            if "--flatbuffer" in test.command_line:
                test.skip = True
                test.skip_reason = "This is a flatbuffer test, can be run with --include_flatbuffers flag"

    if args.for_flatbuffers:
        to_flatbuff_bin = find_to_flatbuf_binary(
            test_base_ref_dir, args.to_flatbuff_path
        )
        if to_flatbuff_bin is None:
            print(
                "Can't find to_flatbuff binary. Did you build the 'to_flatbuff' target?"
            )
            sys.exit(1)
        tests = convert_tests_for_flatbuffers(
            tests, to_flatbuff_bin, test_base_working_dir, color_enum
        )

    if args.skip_network_tests:
        for test in tests:
            if (
                "--active" in test.command_line
                or "--sendto" in test.command_line
                or "--daemon" in test.command_line
            ):
                test.skip = True
                test.skip_reason = "Tests requiring daemon or network skipped because of --skip_network_tests argument"

    tasks: List[Future[TestOutcome]] = []
    completed_tests = Completion()

    executor = ThreadPoolExecutor(max_workers=1)

    for test in tests:
        tasks.append(
            executor.submit(
                run_command_line_test,
                test,
                overwrite=args.overwrite,
                epsilon=args.epsilon,
                base_working_dir=test_base_working_dir,
                ref_dir=test_base_ref_dir,
                completed_tests=completed_tests,
                fuzzy_compare=args.fuzzy_compare,
                valgrind=args.valgrind,
                timeout=args.timeout,
            )
        )

    num_success = 0
    num_fail = 0
    num_skip = 0
    while len(tasks) > 0:
        try:
            outcome: TestOutcome = tasks[0].result()
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

        success_text = f"{color_enum.LIGHT_GREEN}Success{color_enum.ENDC}"
        fail_text = f"{color_enum.LIGHT_RED}Fail{color_enum.ENDC}"
        skipped_text = f"{color_enum.LIGHT_CYAN}Skip{color_enum.ENDC}"
        num_success += outcome.result == Result.SUCCESS
        num_fail += outcome.result == Result.FAIL
        num_skip += outcome.result == Result.SKIPPED

        if outcome.result == Result.SUCCESS:
            result_text = success_text
        elif outcome.result == Result.FAIL:
            result_text = fail_text
        elif outcome.result == Result.SKIPPED:
            skip_reason_text = (
                outcome.skip_reason
                if outcome.skip_reason is not None
                else "unknown reason"
            )
            result_text = f"{skipped_text} ({skip_reason_text})"

        print(f"Test {outcome.id}: {result_text}")
        if outcome.result != Result.SUCCESS:
            test = get_test(outcome.id, tests)
            # Since this test produced a result - it must be in the tests list
            assert test is not None
            print(f"\tDescription: {test.description}")
            print(
                '\t{} _command: "{}"'.format(
                    "bash" if test.is_shell else "vw", test.command_line
                )
            )
        for name, check in outcome.checks.items():
            # Don't print exit_code check as it is too much noise.
            if check.success and name == "exit_code":
                continue
            print(
                "\t[{}] {}: {}".format(
                    name,
                    success_text if check.success else fail_text,
                    check.message,
                )
            )
            if not check.success:
                if name == "exit_code":
                    exit_code_check = cast(StatusCheck, outcome.checks["exit_code"])
                    print("---- stdout ----")
                    print(exit_code_check.stdout)
                    print("---- stderr ----")
                    print(exit_code_check.stderr)

                if isinstance(check, DiffCheck):
                    print()
                    print_colored_diff(check.diff, color_enum)
                    print()

                if args.exit_first_fail:
                    for task in tasks:
                        task.cancel()
                    sys.exit(1)
    print("-----")
    print(f"# Success: {num_success}")
    print(f"# Fail: {num_fail}")
    print(f"# Skip: {num_skip}")

    if num_fail > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
