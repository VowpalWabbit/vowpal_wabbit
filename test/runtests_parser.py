import json
from os import path


class Test:
    output = {}
    unique_id = set()

    @staticmethod
    def register_output(idnum, filename, overwrite=True):
        if not overwrite and Test.has_output(filename):
            return

        Test.output[filename] = idnum

    @staticmethod
    def has_output(filename):
        return filename in Test.output

    @staticmethod
    def filename_to_test_id(filename):
        return Test.output[filename]

    @staticmethod
    def check_add_unique_id(idnum):
        if idnum in Test.unique_id:
            raise Exception("test id is repeated. fatal id: " + str(idnum))
        else:
            Test.unique_id.add(idnum)

    def __init__(self, id, desc):
        self.id = id
        self.desc = desc.strip()
        self.backslash_seen = False
        self.vw_command = ""
        self.is_bash_command = False
        self.diff_files = []

    def add_more_comments(self, line):
        self.desc = self.desc + ". " + line.strip()[1:].strip()

    def add_vw_command(self, line):
        self.vw_command = (self.vw_command + " " + line).strip()
        if self.vw_command[-1] == "\\":
            self.vw_command = self.vw_command[:-1]
            self.backslash_seen = True

        if "&&" in self.vw_command:
            self.is_bash_command = True

    # a cmd is incomplete if self.backslash_seen is true
    # this represents that the previous command parsed ended in '\' char
    def append_cmd_if_incomplete(self, line):
        if self.backslash_seen:
            self.backslash_seen = False
            self.add_vw_command(line)
            return True
        else:
            return False

    def add_file(self, line):
        self.diff_files.append(line)

    def add_bash_command(self, line):
        if self.vw_command:
            raise Exception(
                "construction faulty merging" + self.vw_command + " and " + line
            )
        self.is_bash_command = True
        self.add_vw_command(line)

    def clean(self):
        if not self.is_bash_command:
            self.vw_command = " ".join(self.vw_command.split()[1:])

            # check what input files this command needs
            input_file_flags = [
                "-d",
                "-i",
                "--dictionary",
                "--feature_mask",
                "--cache_file",
            ]
            files = []
            for flag in input_file_flags:
                next_files = Parser.get_values_of_vwarg(self.vw_command, flag)

                # some cleanup only when its dictionary
                if flag == "--dictionary":
                    next_files = [f.split(":")[-1] for f in next_files]
                    dpath = Parser.get_values_of_vwarg(
                        self.vw_command, "--dictionary_path"
                    )
                    if dpath:
                        next_files = [dpath[0] + "/" + f for f in next_files]

                files = files + next_files
            if files:
                self.input_files = files

            # get output files and register as creator of files
            files = Parser.get_values_of_vwarg(self.vw_command, "-f")
            for f in files:
                Test.register_output(int(self.id), f)

            files = Parser.get_values_of_vwarg(self.vw_command, "--cache_file")
            for f in files:
                # treat cache_file differently because it is both an input and output
                Test.register_output(int(self.id), f, overwrite=False)

            # check who produces the input files of this test
            files = Parser.get_values_of_vwarg(self.vw_command, "-i")
            files += Parser.get_values_of_vwarg(self.vw_command, "--feature_mask")
            depends_on = []
            for f in files:
                if "model-sets" not in f:
                    depends_on.append(Test.filename_to_test_id(f))

            files = Parser.get_values_of_vwarg(self.vw_command, "--cache_file")
            for f in files:
                parent_test = Test.filename_to_test_id(f)
                if str(parent_test) != self.id:
                    depends_on.append(parent_test)
                else:
                    self.input_files.remove(f)

            if depends_on:
                self.depends_on = depends_on

        orig_files = self.diff_files

        self.diff_files = {}
        for f in orig_files:
            self.diff_files[Parser.parse_filetype(f)] = f

        self.id = int(self.id)
        Test.check_add_unique_id(self.id)

        if self.is_bash_command:
            self.bash_command = self.vw_command
            delattr(self, "vw_command")

        delattr(self, "is_bash_command")
        delattr(self, "backslash_seen")


class Parser:
    def __init__(self):
        self.curr_test = None
        self.results = []
        self.saw_first_test = False

    @staticmethod
    def get_values_of_vwarg(command, argname):
        command = command.split()
        results = []

        prev_is_argname = False
        for c in command:
            if prev_is_argname:
                results.append(c)
                prev_is_argname = False
            if argname == c:
                prev_is_argname = True

        return results

    @staticmethod
    def parse_filetype(file):
        tokens = file.split(".")
        if tokens[-1] in ["stderr", "stdout"]:
            return tokens[-1]
        else:
            tokens = file.split("/")
            return tokens[-1]

    @staticmethod
    def is_perl_comment(line):
        return line[0] == "#"

    @staticmethod
    def begins_with_vw_command(line):
        return line.split()[0] == "{VW}"

    @staticmethod
    def is_filename_of_testset(line):
        tokens = line.split("/")
        return tokens[0] in ["train-sets", "pred-sets", "test-sets"]

    # RunTests has the contract of ending with this perl commment
    @staticmethod
    def is_last_comment(line):
        return "Do not delete this line or the empty line above it" in line

    # returns a Test(n, ...) instance if line has format:
    # '# Test n:'...
    @staticmethod
    def try_parse_test_definition(line):
        tokens = line.split(":")

        if len(tokens) <= 1:
            return None

        if "# Test" in tokens[0]:
            test_id = tokens[0].split()[-1]
            test_desc = ":".join(tokens[1:])
            return Test(test_id, test_desc)
        else:
            return None

    def commit_parsed_test(self):
        if self.curr_test is not None:
            self.curr_test.clean()
            self.results.append(self.curr_test)
            self.curr_test = None

    def process_line(self, line):
        line = line.strip()

        if line.isspace() or not line:
            return

        if not self.saw_first_test and "# Test 1:" in line:
            self.saw_first_test = True

        if not self.saw_first_test:
            return  # do nothing until we find first test definition

        if Parser.is_perl_comment(line):
            new_test = Parser.try_parse_test_definition(line)

            if new_test:  # we reached a perl comment that declares a new test
                self.commit_parsed_test()
                self.curr_test = new_test
            elif not self.is_last_comment(line):  # its any other perl comment
                self.curr_test.add_more_comments(line)
        elif self.curr_test.append_cmd_if_incomplete(
            line
        ):  # check case if previous line ended in \
            pass
        elif Parser.begins_with_vw_command(line):
            self.curr_test.add_vw_command(line)
        elif Parser.is_filename_of_testset(line):
            self.curr_test.add_file(line)
        else:
            self.curr_test.add_bash_command(line)

    def get_results(self):
        self.commit_parsed_test()
        return self.results


def file_to_obj(filename):
    RTParser = Parser()

    with open(filename) as f:
        for line in f:
            RTParser.process_line(line)

    results = RTParser.get_results()

    # check for missing ids
    i = 1
    for r in results:
        if i != r.id:
            raise Exception("test id being skipped: " + str(i))
        i += 1

    return results


def find_runtest_file():
    rtfile = None

    possible_paths = [path.join(path.dirname(path.abspath(__file__)), "RunTests")]

    for p in possible_paths:
        if path.exists(p):
            rtfile = p

    return rtfile


def main():
    rtfile = find_runtest_file()
    results = file_to_obj(rtfile)

    with open(path.join(path.dirname(rtfile), "runtests.AUTOGEN.json"), "w") as f:
        f.write(json.dumps(results, indent=2, default=lambda x: x.__dict__))


if __name__ == "__main__":
    main()
