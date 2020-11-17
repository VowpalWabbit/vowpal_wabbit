import argparse
import difflib
from pathlib import Path
import re
import os
import errno
import subprocess
import sys
import hashlib

from typing import Optional
import asyncio
import json
import os.path
from concurrent.futures import ThreadPoolExecutor


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


def try_decode(binary_object: Optional[bytes]) -> Optional[str]:
    return binary_object.decode("utf-8") if binary_object is not None else ""


def fuzzy_float_compare(f1, f2, epsilon):
    return (abs(float(f1)-float(f2)) < epsilon)


def is_vw_binary(file_path):
    file_name = os.path.basename(file_path)
    return file_name == "vw"


def find_vw(paths):
    for path in paths:
        absolute_path = os.path.abspath(path)
        if os.path.isdir(absolute_path):
            for file in os.listdir(absolute_path):
                absolute_file = os.path.join(absolute_path, file)
                if is_vw_binary(absolute_file):
                    return absolute_file
        elif os.path.isfile(absolute_path):
            if is_vw_binary(absolute_path):
                return absolute_path
        else:
            # path does not exist
            continue
    raise ValueError("none found.")


def line_diff_text(text_one, file_name_one, text_two, file_name_two):
    text_one = [line.strip() for line in text_one.strip().splitlines()]
    text_two = [line.strip() for line in text_two.strip().splitlines()]

    diff = difflib.unified_diff(
        text_two, text_one, fromfile=file_name_two, tofile=file_name_one, lineterm='')
    output_lines = []
    for line in diff:
        output_lines.append(line)

    return len(output_lines) != 0, output_lines


def compare_line(output_line, ref_line, epsilon):
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


def compare_lines(output_lines, ref_lines, epsilon):
    if len(output_lines) != len(ref_lines):
        return True, "Diff mismatch"

    found_close_floats = False
    for output_line, ref_line in zip(output_lines, ref_lines):
        # Some output contains '...', remove this for comparison.
        output_line = output_line.replace("...", "")
        ref_line = ref_line.replace("...", "")
        is_different, reason, found_close_floats_temp = compare_line(
            output_line, ref_line, epsilon)
        found_close_floats = found_close_floats or found_close_floats_temp
        if is_different:
            return True, reason

    return False, "Minor float difference ignored" if found_close_floats else ""


def is_diff_different(output_content, output_file_name, ref_content, ref_file_name, epsilon):
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

    is_different, reason = compare_lines(output_lines, ref_lines, epsilon)
    diff = diff if is_different else []
    return is_different, diff, reason


def are_outputs_different(output_content, output_file_name, ref_content, ref_file_name, overwrite, epsilon):
    is_different, diff, reason = is_diff_different(
        output_content, output_file_name, ref_content, ref_file_name, epsilon)

    if is_different and overwrite:
        with open(ref_file_name, 'w') as writer:
            writer.write(output_content)

    if not is_different:
        return False, [], reason

    # If diff difference fails, fall back to a line by line compare to double check.

    output_lines = [line.strip()
                    for line in output_content.strip().splitlines()]
    ref_lines = [line.strip() for line in ref_content.strip().splitlines()]
    is_different, reason = compare_lines(output_lines, ref_lines, epsilon)
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


async def run_command_line_test(loop, executor, command_line, comparison_files, overwrite, epsilon, dependency=None):
    def inner_thing(command_line, comparison_files, dependency=None):
        result = subprocess.run(
            (f"{command_line}").split(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
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
                if os.path.isfile(output_file):
                    output_content = open(output_file, 'r').read()
                else:
                    checks[output_file] = {
                        "success": False,
                        "message": f"Failed to open output file: {output_file}",
                        "diff": []
                    }
                    continue

            if os.path.isfile(ref_file):
                ref_content = open(ref_file, 'r').read()
            else:
                checks[output_file] = {
                    "success": False,
                    "message": f"Failed to open ref file: {ref_file}",
                    "diff": []
                }
                continue
            are_different, diff, reason = are_outputs_different(output_content, output_file,
                                                                ref_content, ref_file, overwrite, epsilon)

            if are_different:
                message = f"Diff not OK, {reason}"
            else:
                message = f"Diff OK, {reason}"

            checks[output_file] = {
                "success": are_different == False,
                "message": message,
                "diff": diff
            }

        return {
            "success": all(check["success"] == True for name, check in checks.items()),
            "checks": checks
        }

    if dependency is not None:
        await dependency
    return await loop.run_in_executor(executor, inner_thing, command_line, comparison_files, dependency)


SEARCH_PATHS = ["../build/vowpalwabbit", "test",
                "../vowpalwabbit", "vowpalwabbit", "."]


async def main():
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('-t', "--test", type=int)
    parser.add_argument('-E', "--epsilon", type=float, default=1e-4)
    parser.add_argument('-o', "--overwrite", type=bool, default=False)
    args = parser.parse_args()

    vw = find_vw(SEARCH_PATHS)

    loop = asyncio.get_event_loop()
    executor = ThreadPoolExecutor()
    with open("test_spec.json", 'r') as f:
        tests = json.load(f)["tests"]

    test_tasks = []
    named_tasks = {}
    for i, test in enumerate(tests):
        test_number = i + 1
        if args.test is not None and args.test != test_number:
            continue

        if "type" not in test:
            print(
                f"Skipping test number '{test_number}' as 'type' is missing")
            continue

        elif test["type"] == "vw_cmd_line" or test["type"] == "bash_script":
            dependency = None
            if "depends_on" in test:
                depends_on_id = test["depends_on"]
                if depends_on_id not in named_tasks:
                    print(
                        f"No task with id '{depends_on_id}' exists, specify which task is '{depends_on_id}' with `\"id\":\"{depends_on_id}\"")
                    sys.exit(1)
                dependency = named_tasks[test["depends_on"]]

            if test["type"] == "vw_cmd_line":
                command_line = f"{vw} {test['command_line']}"
            elif test["type"] == "bash_script":
                if sys.platform == "win32":
                    print(
                        f"Skipping test number '{test_number}' as bash_script is an unsupported type on Windows.")
                    continue
                command_line = test['command_line'].format(vw=vw)

            this_task = asyncio.create_task(run_command_line_test(
                loop, executor, command_line, test["comparison_files"], overwrite=args.overwrite, epsilon=args.epsilon, dependency=dependency))
            if "id" in test:
                this_id = test["id"]
                if this_id in named_tasks:
                    print(f"There already exists a test with name '{this_id}'")
                named_tasks[this_id] = this_task
            test_tasks.append((test_number, this_task))
        else:
            print(
                f"Skipping unknown test type '{test['type']}' for test number '{test_number}'")
            continue

    num_success = 0
    num_fail = 0
    for i, task in test_tasks:
        result = await task
        success_text = f"{Color.OKGREEN}Success{Color.ENDC}"
        fail_text = f"{Color.FAIL}Fail{Color.ENDC}"
        num_success += 1 if result['success'] else 0
        num_fail += 0 if result['success'] else 1
        print(f"Test {i}: {success_text if result['success'] else fail_text}")
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
    print(f"-----")
    print(f"# Success: {num_success}")
    print(f"# Fail: {num_fail}")


if __name__ == "__main__":
    asyncio.run(main())
