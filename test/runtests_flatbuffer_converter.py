import copy
import os
import os.path
import shlex
import subprocess
from pathlib import Path
import re
from typing import List, Optional, Tuple

from run_tests_common import TestData


def remove_first_arg(command_line: str) -> str:
    return " ".join(command_line.split()[1:])


def is_input_data_file(input_file):
    return "train-set" in input_file or "test-set" in input_file


class FlatbufferTest:
    def __init__(
        self, test: TestData, working_dir, depends_on_test: Optional[TestData] = None
    ):
        self.test = test
        self.working_dir = working_dir
        self.stashed_input_files = copy.copy(self.test.input_files)
        self.stashed_vw_command = copy.copy(self.test.command_line)
        self.test_id = str(self.test.id)
        self.files_to_be_converted: List[Tuple[int, str, str]] = []
        self.depends_on_cmd = (
            copy.copy(depends_on_test.stashed_command_line)
            if (depends_on_test is not None)
            else None
        )

        # self.depends_on_cmd = depends_on_cmd

        test_dir = self.working_dir.joinpath("test_" + self.test_id)
        if not Path(str(test_dir)).exists():
            Path(str(test_dir)).mkdir(parents=True, exist_ok=True)

    def remove_arguments(self, command, tags_delete, flags=False):
        for tag in tags_delete:
            if flags:
                command = re.sub(tag, "", command)
            else:
                command = re.sub(r"{} [:a-zA-Z0-9_.\-/]*".format(tag), "", command)
        return command

    def get_flatbuffer_file_names(self):
        for i, input_file in enumerate(self.test.input_files):
            if is_input_data_file(input_file):
                file_basename = os.path.basename(input_file)
                fb_file = "".join([file_basename, ".fb"])
                fb_file_full_path = self.working_dir.joinpath(
                    "test_" + self.test_id
                ).joinpath(fb_file)
                self.files_to_be_converted.append(
                    (i, str(input_file), str(fb_file_full_path))
                )

    def replace_line(self, line):
        for index, from_file, to_file in self.files_to_be_converted:
            line = line.replace(from_file, to_file)
        return line

    def replace_filename_in_stderr(self):
        if "stderr" in self.test.comparison_files:
            stderr_file = self.test.comparison_files["stderr"]
            stderr_test_file = str(
                self.working_dir.joinpath("test_" + self.test_id).joinpath(
                    os.path.basename(str(self.working_dir.joinpath(stderr_file)))
                )
            )
            with open(stderr_file, "r") as f, open(stderr_test_file, "w") as tmp_f:
                contents = [self.replace_line(line) for line in f]
                for line in contents:
                    tmp_f.write(line)

            self.test.comparison_files["stderr"] = str(stderr_test_file)

    def replace_test_input_files(self):
        # replace the input_file to point to the generated flatbuffer file
        for i, from_file, to_file in self.files_to_be_converted:
            self.test.input_files[i] = to_file

    def convert(self, to_flatbuff, color_enum):
        # arguments and flats not supported or needed in flatbuffer conversion
        flags_to_remove = [
            "-c ",
            "--bfgs",
            "--onethread",
            "-t ",
            "--search_span_bilou",
        ]
        arguments_to_remove = [
            "--passes",
            "--ngram",
            "--skips",
            "-q",
            "-p",
            "--feature_mask",
            "--search_kbest",
            "--search_max_branch",
        ]

        # if model already exists it contains needed arguments so use it in conversion
        use_model = False
        for input_file in self.stashed_input_files:
            if "model-set" in input_file:
                use_model = True

        if not use_model:
            arguments_to_remove.append("-i")  # lose the model input

        # The command line already has the vw binary prepended - so snip it off for this
        to_flatbuff_command = (
            remove_first_arg(self.depends_on_cmd)
            if self.depends_on_cmd is not None
            else remove_first_arg(self.test.command_line)
        )
        to_flatbuff_command = self.remove_arguments(
            to_flatbuff_command, arguments_to_remove
        )
        to_flatbuff_command = self.remove_arguments(
            to_flatbuff_command, flags_to_remove, flags=True
        )

        for i, from_file, to_file in self.files_to_be_converted:
            # replace depends_on filename with our filename, will do nothing if no depends_on
            to_flatbuff_command = re.sub(
                r"{} [:a-zA-Z0-9_.\-/]*".format("-d"),
                "",
                to_flatbuff_command,
            )
            to_flatbuff_command = f"-d {from_file} " + to_flatbuff_command

            cmd = "{} {} {} {}".format(
                to_flatbuff, to_flatbuff_command, "--fb_out", to_file
            )
            if self.depends_on_cmd is not None and "audit" in self.test.command_line:
                cmd += " --audit"
            print(
                "{}CONVERT COMMAND {} {}{}".format(
                    color_enum.LIGHT_PURPLE, self.test_id, cmd, color_enum.ENDC
                )
            )
            result = subprocess.run(shlex.split(cmd), check=True)
            if result.returncode != 0:
                raise RuntimeError(
                    "Generating flatbuffer file failed with {} {} {}".format(
                        result.returncode, result.stderr, result.stdout
                    )
                )

    def replace_vw_command(self):
        # restore original command in case it changed
        # but add new field to the test with the original vw command
        self.test.command_line = self.stashed_vw_command
        self.test.stashed_command_line = self.stashed_vw_command

        # remove json/dsjson since we are adding --flatbuffer
        json_args = ["--json", "--dsjson"]
        if "--dsjson" in self.test.command_line:
            json_args.append("--chain_hash")
        self.test.command_line = self.remove_arguments(
            self.test.command_line, json_args, flags=True
        )

        # replace data files with flatbuffer ones in vw_command
        for i, from_file, to_file in self.files_to_be_converted:
            self.test.command_line = self.test.command_line.replace(from_file, to_file)
        # add --flatbuffer argument
        self.test.command_line = self.test.command_line + " --flatbuffer"

    def to_flatbuffer(self, to_flatbuff, color_enum):
        self.get_flatbuffer_file_names()
        self.replace_filename_in_stderr()
        self.replace_test_input_files()
        self.convert(to_flatbuff, color_enum)
        self.replace_vw_command()
