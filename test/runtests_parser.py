import json
from os import path

class Test:
    output = {}
    unique_id = set()

    @staticmethod
    def register_output(idnum, filename):
        Test.output[filename] = idnum
    
    @staticmethod
    def filename_to_test_id(filename):
        return Test.output[filename]

    @staticmethod
    def check_add_unique_id(idnum):
        if idnum in Test.unique_id:
            raise Exception("test id is repeated. fatal id: " + idnum)
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
            raise Exception("construction faulty merging" + self.vw_command + " and " + line)
        self.is_bash_command = True
        self.add_vw_command(line)

    def clean(self):
        if not self.is_bash_command:
            self.vw_command = " ".join(self.vw_command.split()[1:])

            # check what input files this command needs
            files = Parser.get_values_of_vwarg(self.vw_command, "-d")
            files = files + Parser.get_values_of_vwarg(self.vw_command, "-i")
            files = files + Parser.get_values_of_vwarg(self.vw_command, "--dictionary")
            files = files + Parser.get_values_of_vwarg(self.vw_command, "--feature_mask")
            if files:
                self.input_files = files

            # get output files and register as creator of files
            files = Parser.get_values_of_vwarg(self.vw_command, "-f")
            for f in files:
                Test.register_output(int(self.id), f)

            # check who produces the input files of this test
            files = Parser.get_values_of_vwarg(self.vw_command, "-i")
            depends_on = []
            for f in files:
                if "model-sets" not in f:
                    depends_on.append(Test.filename_to_test_id(f))
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
            delattr(self, 'vw_command')

        delattr(self, 'is_bash_command')
        delattr(self, 'backslash_seen')


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

    # returns a Test(n, ...) instance if line has format:
    # '# Test n:'...
    @staticmethod
    def try_parse_test_definition(line):
        tokens = line.split(":")

        if len(tokens) <= 1:
            return None
        
        if "# Test" in tokens[0]:
            test_id = tokens[0].split()[-1]
            test_desc = ':'.join(tokens[1:])
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
            return # do nothing until we find first test definition

        if Parser.is_perl_comment(line):
            new_test = Parser.try_parse_test_definition(line)

            if new_test: # we reached a perl comment that declares a new test
                self.commit_parsed_test()
                self.curr_test = new_test
            else: # its any other perl comment
                self.curr_test.add_more_comments(line)
        elif self.curr_test.append_cmd_if_incomplete(line): # check case if previous line ended in \
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
    
    return RTParser.get_results()

def main():
    possible_paths = ["./RunTests", "./test/RunTests"]
    for p in possible_paths:
        if path.exists(p):
            rtfile = p

    results = file_to_obj(rtfile)

    with open(path.join(path.dirname(rtfile), "runtests.AUTOGEN.json"), "w") as f:
        f.write(json.dumps(results, indent=2, default=lambda x: x.__dict__))

if __name__ == "__main__":
    main()