import os
import re

with open('../RunTests', 'r') as file:
    lines = file.read()


def extract_something(lines, something):
    predsets = []
    for i in lines:
        if something in i:
            predsets.append(i.strip())

    return predsets

predsets = extract_something(lines.split('\n'), 'pred-sets/ref')
testsets = extract_something(lines.split('\n'), 'test-sets/ref')
trainsets = extract_something(lines.split('\n'), 'train-sets/ref')
models = extract_something(lines.split('\n'), 'models')
modelsets = extract_something(lines.split('\n'), 'model-sets')

def extract_datafile(command):
    datafilename = re.search('-d [/a-zA-Z0-9_.-]*', command)
    if datafilename is not None:
        return datafilename.group(0)

    else:
        return None

def present(tags, command):
    for i in tags:
        if i in command:
            return 1
        
    return 0

def delete(command, tags_delete, only_keyword=False):
    for tag in tags_delete:
        if only_keyword:
            command = re.sub(tag, '', command)
        else:
            command = re.sub('{} [:a-zA-Z0-9_.-]*'.format(tag), '', command)
    return command


def extract_commands(lines, tags, tags_delete):
    lines = lines.split('__DATA__')[-1]
    commands = lines.split('\n# Test ')[1:]
    e_commands, names, datafiles = [], [], {}
    no_to_command = {}
    for command in commands:
        com = command.split('\n')
        names.append(com[0])
        for i, c in enumerate(com):
            if '{VW}' in c:
                index = i
                break
        if '\\' in com[index]:
            com = ''.join([i.strip('\\') for i in com[index:] if '/ref/' not in i ])
            com = " ".join(com.split())
        else:
            com = com[index]
        if com[:4] != '{VW}':
            continue
        # print(com)
        com = com.replace('{VW}', '../../build/utl/flatbuffer/to_flatbuff')
        com = com.replace('# Do not delete this line or the empty line above it', '')

        com = delete(com, tags_delete)
        com = delete(com, tags, only_keyword=True)

        test_id = names[-1].split(':')[0]
        datafiles[test_id] = extract_datafile(com)

        no_to_command[test_id] = com

    return names, e_commands, no_to_command, datafiles


def search_and_replace(command, string):
    if string is not None:
        command = re.sub(string, '-d ../{}'.format(string.split(' ')[1]), command)
    
    return command

def add_flatout(command, string):
    if string is not None:
        command += ' --flatout {}'.format(string.split(' ')[1])

    return command

def change_datafile_names(commands, datafiles):
    newc = {}
    for k, v in commands.items():
        command = search_and_replace(v, datafiles[k])
        command = add_flatout(command, datafiles[k])
        newc[k] = command

    return newc

def skip(commands, to_skip):
    newc = {}
    for k, v in commands.items():
        if int(k) not in to_skip:
            newc[k] = v

    return newc

tags = ['--audit', '-c ','--bfgs']
td = ['--passes', '--ngram', '--skips', '-q', '-b']

names, _c, _d, datafiles = extract_commands(lines, tags, td)

commands = change_datafile_names(_d, datafiles)
to_skip = [60, 61, 92, 96, 212, 213, 214, 215, 216, 217, 218, 219, 83, 88, 89, 210, 198, 202, 209, 83, 176, 177, 203, 206, 207, 208, 193, 194, 195, 155, 156, 152, 149, 137, 125, 111, 112]
commands = skip(commands, to_skip)

with open('create_datafiles.sh', 'w') as file:
    for k, v in commands.items():
        file.write("echo \"{}\" && ".format(k))
        file.write(v)
        file.write('\n')