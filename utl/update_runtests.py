import os

with open('test/RunTests', 'r') as file:
    text = file.read()

lines = text.split('# Test 1:')[1]
commands = lines.split('\n\n')
# print(len(commands))
to_skip = ['60','61','92','96','149','152', '156', '176', '177', '203', '204', 'Do not delete this line or the empty line above it']

def should_skip(command, to_skip):
    for j in to_skip:
        if j in command:
            return True

    return False

files = []
stderrs = []
for line in lines.split('\n'):
    if 'stderr' in line:
        stderrs.append(line)
    # else:
    #     print("Could not find stderr file for )

# print(len(stderrs), len(commands))

def return_relevant_stderr(text, stderrs):
    for stderr in stderrs:
        if stderr in text:
            return stderr
    

stderr_to_data = {}

for command in commands:
    if should_skip(command, to_skip):
        continue

    # print(command)

    command = command.strip('\n')
    command = command.strip('\t')
    command = command.strip('\\')
    text = command.split('-d ')[1].split(' ')
    for i, t in enumerate(text):
        if i==0:
            file = t
        # if 'stderr' in t:
        #     stderrs.append(t)
    stderr = return_relevant_stderr(command, stderrs)

    file = file.strip('\n')
    file = file.strip('\t')
    file = file.strip('\\')
    file = file.strip()
    files.append(file)
    # print(stderr, file)
    if stderr:
        stderr_to_data[stderr.strip()] = file

files = set(files)

count = 0
flat = 0
stderr = 0
for file in files:
    path = '{}/{}'.format('test', file)
    if os.path.exists(path):
        count += 1

    flatbuffer = '{}.fb'.format(path)
    if os.path.exists(flatbuffer):
        flat += 1


for file in stderrs:
    path = '{}/{}'.format('test', file.strip())
    if os.path.exists(path):
        stderr += 1

print("Found {} original data files".format(count))
print("Found {} Converted flatbuffers".format(flat))
print("Found {} stderr files".format(stderr))

for file in stderrs:
    path = '{}/{}'.format('test', file.strip())
    if os.path.exists(path):
        with open(path, 'r') as filer:
            text = filer.read()

        datafile = stderr_to_data[file.strip()]

        text.replace('Reading datafile = {}'.format(datafile), 'Reading datafile = {}.fb'.format(datafile))

        with open('{}_fb.stderr'.format(path.split('stderr')[0]), 'w') as file:
            file.write(text)

# To-do:
# 1. Convert original data files to new flatbuffer files in new folder
# 2.
