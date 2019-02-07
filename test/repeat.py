import subprocess
import difflib
import sys


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
