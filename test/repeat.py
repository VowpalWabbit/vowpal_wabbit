import subprocess
import difflib
import sys

# This file is used to test some race conditions.
# Two race conditions to worry about
#
# Scencario 1: done-then-dispatch
# 1. Parsing thread sets p->done
# 2. learning thread get_example notes end_parsed_examples = used_index
# 3. learning thread early terminates.
# 4. Parsing thread calls dispatch_example.
#
# Scenario 2: dispatch-then-done
# 1. parser thread dispatches example
# 2. learner thread consumes examples
# 3. learner thread hangs on p->example_available
# 4. parser thread sets done to true.
#
# Fix: Use scenario 2 but have done raise the examples_available flag to unblock the learning thread.

count = int(sys.argv[1])
cmd = sys.argv[2:]
counter = 0
next_print = 1
error = 0
expected = None

for counter in range(1, count + 1):
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    out, err = p.communicate('')
    out += err or ''
    if expected is None:
        expected = out
        continue

    failed = False
    if p.wait() != 0:
        failed = True
        sys.stderr.write("Return code: %s\n" % p.wait())

    if expected != out:
        failed = True
        sys.stderr.write('\nOutput changed:\n\n')
        d = difflib.Differ()
        diff = d.compare(expected.split('\n'), out.split('\n'))
        sys.stderr.write('\n'.join(diff) + '\n')

    if failed:
        error += 1

    if counter >= next_print or failed:
        sys.stderr.write("%s failed out of%5s\n" % (error, counter))
        if counter >= next_print:
            next_print *= 2

    if error:
        break


if error:
    sys.exit(1)
