import fileinput
import copy
import os
import os.path
import subprocess
from enum import Enum
import shutil
from pathlib import Path
import re

class FlatbufferTest:
    def __init__(self, test, working_dir):
        self.test = test
        self.working_dir = working_dir
        self.stashed_input_files = copy.copy(self.test['input_files'])
        self.stashed_vw_command = copy.copy(self.test['vw_command'])
        self.test_id = str(self.test['id'])
        self.files_to_be_transformed = []
        self.input_files = self.test['input_files']
        self.fb_input_files_full_path = []
        self.fb_input_files = []

        test_dir = self.working_dir.joinpath('test_' + self.test_id)
        if not Path(str(test_dir)).exists():
            Path(str(test_dir)).mkdir(parents=True, exist_ok=True)
    
    def remove_arguments(self, command, tags_delete, flags=False):
        for tag in tags_delete:
            if flags:
                command = re.sub(tag, '', command)
            else:
                command = re.sub('{} [:a-zA-Z0-9_.\-/]*'.format(tag), '', command)
        return command

    def change_input_file(self, input_file):
        return 'train-set' in input_file or 'test-set' in input_file

    def get_flatbuffer_file_names(self):
        for i, input_file in enumerate(self.input_files):
            if self.change_input_file(input_file):
                file_basename = os.path.basename(input_file)
                fb_file = ''.join([file_basename, '.fb'])
                self.fb_input_files.append(fb_file)
                fb_file_full_path = self.working_dir.joinpath('test_' + self.test_id).joinpath(fb_file)
                self.fb_input_files_full_path.append(fb_file_full_path)

    def replace_filename_in_stderr(self):
        if 'stderr' in self.test['diff_files']:
            stderr_file = self.test['diff_files']['stderr']
            stderr_test_file = self.working_dir.joinpath('test_' + self.test_id).joinpath(os.path.basename(self.working_dir.joinpath(stderr_file)))
            shutil.copyfile(stderr_file, str(stderr_test_file))        
            temp = str(stderr_test_file) + '.bak'
            with open(stderr_test_file, 'r') as f:
                with open(temp, 'w') as tmp_f:
                    for line in f:
                        for i, input_file in enumerate(self.input_files):
                            if self.change_input_file(input_file):
                                line = line.replace(str(input_file), str(self.fb_input_files_full_path[i]))
                        tmp_f.write(line)
            
            # swap temp with file
            shutil.move(temp, stderr_test_file)

            self.test['diff_files']['stderr'] = str(stderr_test_file)
    
    def replace_test_input_files(self):
        # replace the input_file to point to the generated flatbuffer file
        for i, input_file in enumerate(self.input_files):
            if self.change_input_file(input_file):
                self.test['input_files'][i] = str(self.fb_input_files_full_path[i])
                self.files_to_be_transformed.append(str(self.fb_input_files_full_path[i]))
    
    def should_transform(self):
        if 'depends_on' in self.test: # assuming dependent tests use same input files, so might already be transformed
            # check if the file exists in the dependant directories
            for f in self.files_to_be_transformed:
                search_paths = []
                dependencies = self.test['depends_on']
                search_paths.extend([self.working_dir.joinpath(
                    "test_{}".format(x), os.path.basename(f)) for x in dependencies]) # for input_files with a full path

                for search_path in search_paths:
                    if search_path.exists() and not search_path.is_dir():
                        return False
        return True
    
    def to_flatbuffer(self, to_flatbuff, color_enum):
        # arguments and flats not supported or needed in flatbuffer transformation
        flags_to_remove = ['--audit', '-c ','--bfgs', '--onethread', '-t ', '--search_span_bilou']
        arguments_to_remove = ['--passes', '--ngram', '--skips', '-q', '-p', '--feature_mask', '--search_kbest', '--search_max_branch']

        # if model already exists it contains needed arguments so use it in transformation
        use_model = False
        for input_file in self.stashed_input_files:
            if 'model-set' in input_file:
                use_model = True
        
        if not use_model:
            arguments_to_remove.append('-i') # loose the model input
            
        to_flatbuff_command = self.test['vw_command']
        to_flatbuff_command = self.remove_arguments(to_flatbuff_command, arguments_to_remove)
        to_flatbuff_command = self.remove_arguments(to_flatbuff_command, flags_to_remove, flags=True)

        if self.should_transform(): # transformation might already be done by depended_on test
            for f in self.files_to_be_transformed:
                cmd = "{} {} {} {}".format(to_flatbuff, to_flatbuff_command, '--fb_out', f)
                print("{}COMMAND {} {}{}".format(color_enum.LIGHT_PURPLE, self.test_id, cmd, color_enum.ENDC))
                result = subprocess.run(
                    cmd,
                    shell=True,
                    check=True)
                if result.returncode != 0:
                    raise RuntimeError("Generating flatbuffer file failed with {} {} {}".format(result.returncode, result.stderr, result.stdout))

        
    def replace_vw_command(self):
        # restore original command in case it changed
        self.test['vw_command'] = self.stashed_vw_command

        # remove json/dsjson since we are adding --flatbuffer
        json_args = ['--json', '--dsjson', '--chain_hash']
        self.test['vw_command'] = self.remove_arguments(self.test['vw_command'], json_args, flags=True)

        # replace data files with flatbuffer ones in vw_command
        for i, input_file in enumerate(self.stashed_input_files):
            if self.change_input_file(input_file):
                self.test['vw_command'] = self.test['vw_command'].replace(str(input_file), str(self.fb_input_files_full_path[i]))
        # add --flatbuffer argument
        self.test['vw_command'] = self.test['vw_command'] + ' --flatbuffer'