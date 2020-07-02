import os

# files = os.listdir('../train-sets/')

# flatbuffers = [i for i in files if '.fb' in i and '.fb.cache' not in i]

# for i in flatbuffers:
#     os.system('cp ../train-sets/{} train-sets/{}'.format(i, i[:-3]))

with open('RunTests', 'r') as file:
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

print(len(predsets) + len(testsets) + len(trainsets))

# for i in predsets:
#     os.system('cp ../{} {}'.format(i, i))
# for i in trainsets:
#     os.system('cp ../{} {}'.format(i, i))
# for i in testsets:
#     os.system('cp ../{} {}'.format(i, i))

def extract_commands(lines):
    lines = lines.split('__DATA__')[-1]
    commands = lines.split('\n# Test')
