"""
Test that the models generated with and without --predict_only_model produce the same predictions when loaded in test_mode.
"""
import sys
import os
import optparse
import random
from itertools import zip_longest


def system(cmd, verbose=True):
    cmd = cmd.replace("VW", VW)
    if verbose:
        sys.stderr.write("+ %s\n" % cmd)
    retcode = os.system(cmd)
    if retcode:
        sys.exit(1)


def read_output(cmd, verbose=True):
    cmd = cmd.replace("VW", VW)
    if verbose:
        sys.stderr.write("+ %s\n" % cmd)
    p = os.popen(cmd)
    out = p.read()
    if p.close():
        sys.exit(1)
    return out


def unlink(filename):
    if os.path.exists(filename):
        os.unlink(filename)


def get_file_size(filename, cache={}):
    if filename in cache:
        return cache[filename]
    file_size = int(os.popen("wc -l < %s" % filename).read())
    cache[filename] = file_size
    return file_size


def choice_no_replacement(a, size):
    result = list(a)
    random.shuffle(result)
    return result[:size]


def do_test(filename, args, verbose=None, repeat_args=None, known_failure=False):
    if isinstance(args, list):
        args = " ".join(args)

    if verbose is None:
        verbose = globals()["verbose"]

    if repeat_args is None:
        repeat_args = globals()["repeat_args"]

    file_size = get_file_size(filename)
    if verbose:
        sys.stderr.write(
            "Testing %s %s on %s (%s lines)\n" % (VW, args, filename, file_size)
        )
    splits = choice_no_replacement(range(1, file_size - 2), size=10)
    splits.sort()
    splits = list(splits)
    if verbose:
        sys.stderr.write("Using splits: %s\n" % splits)

    tmp_model = "tmp.save_resume_test.%s" % os.getpid()

    resume_args = args if repeat_args else ""

    try:
        for index, split in enumerate(splits[:-1]):
            try:
                system(
                    "head -n %s %s | VW %s -f %s.full --quiet"
                    % (split, filename, args, tmp_model),
                    verbose=verbose,
                )

                if index:
                    system(
                        "head -n %s %s | tail -n %s | VW --quiet -f %s.resume -i %s.resume %s"
                        % (
                            split,
                            filename,
                            split - splits[index - 1],
                            tmp_model,
                            tmp_model,
                            resume_args,
                        ),
                        verbose=verbose,
                    )
                else:
                    system(
                        "head -n %s %s | VW %s --quiet -f %s.resume"
                        % (split, filename, args, tmp_model),
                        verbose=verbose,
                    )

                predictions_normal = read_output(
                    "head -n %s %s | tail -n %s | VW --quiet -i %s.full -t -p /dev/stdout"
                    % (
                        splits[index + 1],
                        filename,
                        splits[index + 1] - split,
                        tmp_model,
                    ),
                    verbose=verbose,
                )
                predictions_resume = read_output(
                    "head -n %s %s | tail -n %s | VW --quiet -i %s.resume -t -p /dev/stdout"
                    % (
                        splits[index + 1],
                        filename,
                        splits[index + 1] - split,
                        tmp_model,
                    ),
                    verbose=verbose,
                )

                for index, (p_normal, p_resume) in enumerate(
                    zip_longest(
                        predictions_normal.split("\n"), predictions_resume.split("\n")
                    )
                ):
                    if p_normal != p_resume:
                        if verbose:
                            sys.stderr.write(
                                "line %s: %r != %r\n" % (index + 1, p_normal, p_resume)
                            )
                        if known_failure:
                            sys.stderr.write("KNOWN FAILURE %s %s\n" % (VW, args))
                            return 0
                        sys.stderr.write(
                            "FAILED %s %s%s\n"
                            % (VW, args, " (known failure)" if known_failure else "")
                        )
                        if not verbose and verbose_on_fail:
                            sys.stderr.write("Redoing with verbose on:\n")
                            do_test(filename, args, verbose=True)
                        return 1
            finally:
                unlink(tmp_model + ".full")
    finally:
        unlink(tmp_model + ".resume")

    if known_failure:
        sys.stderr.write("OK (BUT EXPECTED TO FAIL) %s %s\n" % (VW, args))
        return 1

    sys.stderr.write("OK %s %s\n" % (VW, args))
    return 0


if __name__ == "__main__":
    parser = optparse.OptionParser()
    parser.add_option("--filename", default="train-sets/rcv1_small.dat")
    parser.add_option("--vw", default="../build/vowpalwabbit/cli/vw")
    parser.add_option("--no_repeat_args", action="store_true")
    parser.add_option("--verbose", action="store_true")
    parser.add_option("--verbose_on_fail", action="store_true")
    options, args = parser.parse_args()

    VW = options.vw
    filename = options.filename
    repeat_args = not options.no_repeat_args
    verbose = options.verbose
    verbose_on_fail = options.verbose_on_fail
    errors = 0

    if args:
        errors += do_test(filename, args)
    else:
        errors += do_test(filename, "")
        errors += do_test(filename, "-b 22")
        errors += do_test(filename, "--sparse_weights")
        errors += do_test(filename, "--noconstant")
        errors += do_test(filename, "--constant 10")
        errors += do_test(filename, "--random_seed 55")
        errors += do_test(filename, "--min_prediction 0.1")
        errors += do_test(filename, "--max_prediction 0.1")
        errors += do_test(filename, "--hash_seed 10")
        errors += do_test(filename, "--loss_function logistic")
        # errors += do_test(filename, "--boosting 10") TODO: save_load params_per_model
        # errors += do_test(filename, "--bootstrap 10") TODO: save_load params_per_model
        errors += do_test(filename, "--l1 1e-04")
        errors += do_test(filename, "--l2 1e-04")
        errors += do_test(filename, "--learning_rate 0.1")
        errors += do_test(filename, "--power_t 0.1")
        errors += do_test(filename, "--initial_t 10")
        errors += do_test(filename, "--loss_function quantile")
        errors += do_test(filename, "--loss_function quantile --quantile_tau 0.2")
        errors += do_test(filename, "--sgd")
        errors += do_test(filename, "--adaptive")
        errors += do_test(filename, "--normalized")
        errors += do_test(filename, "--invariant")
        errors += do_test(filename, "--loss_function logistic --link logistic")
        # errors += do_test(filename, "--nn 2") TODO: save_load params_per_model
        errors += do_test(filename, "--binary")
        errors += do_test(filename, "--ftrl")
        errors += do_test(filename, "--pistol")
        errors += do_test(filename, "--coin")

        # this one also fails but pollutes output
        # errors += do_test(filename, '--ksvm', known_failure=True)

        errors += do_test("train-sets/multiclass", "--oaa 10")
        errors += do_test("train-sets/multiclass", "--csoaa 10")
        errors += do_test("train-sets/multiclass", "--ect 10")
        errors += do_test("train-sets/multiclass", "--log_multi 10")
        errors += do_test("train-sets/multiclass", "--recall_tree 10")

    if errors:
        sys.exit("%s failed" % errors)
