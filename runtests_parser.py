import json

class Test:
    output = {}

    @staticmethod
    def register_output(idnum, filename):
        Test.output[filename] = idnum


    def __init__(self, id, desc):   
        self.id = id 
        self.desc = desc.strip()
        self.backslash_seen = False
        self.vw_command = "" 
        self.is_bash_command = False
        self.files = []
        self.depends_on = []

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
        self.files.append(line)

    def add_bash_command(self, line):
        if self.vw_command:
            raise Exception("construction faulty merging" + self.vw_command + " and " + line)
        self.is_bash_command = True
        self.add_vw_command(line)

    @staticmethod
    def get_value_of_arg(command, argname):
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

    def clean(self):
        if not self.is_bash_command:
            self.vw_command = " ".join(self.vw_command.split()[1:])

            # get output files and register as creator of files
            files = Test.get_value_of_arg(self.vw_command, "-f")
            for f in files:
                Test.register_output(int(self.id), f)

            # check who produces the input files of this test
            files = Test.get_value_of_arg(self.vw_command, "-i")
            for f in files:
                if "model-sets" not in f:
                    self.depends_on.append(Test.output[f])

        if self.id.isnumeric():
            self.id = int(self.id)
        else:
            raise Exception("id is not a number. fatal.")

        delattr(self, 'backslash_seen')

class Parser:
    def __init__(self):   
        self.curr_test = None
        self.results = []

    @staticmethod
    def is_perl_comment(line):
        return line[0] == "#"

    @staticmethod
    def begins_with_vw_command(line):
        return line.split()[0] == "{VW}"

    @staticmethod
    def is_filename_of_testset(line):
        tokens = line.split("/")
        if tokens[0] in ["train-sets", "pred-sets", "test-sets"]:
            return True
        
        return False

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
    
    def get_json_str(self):
        self.commit_parsed_test()
        return json.dumps(self.results, indent=2, default=lambda x: x.__dict__)

with open('./test/RunTests') as f:
    saw_first_test = False

    RTParser = Parser()

    for line in f:
        if "# Test 1:" in line:
            saw_first_test = True

        if saw_first_test and not line.isspace():
            RTParser.process_line(line.strip())
    
    print(RTParser.get_json_str())