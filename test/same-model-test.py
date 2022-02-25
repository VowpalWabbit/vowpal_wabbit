import sys
import argparse
import subprocess

SPANNING_TREE_PORT = 26546

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--vw", help="Path to VW binary to use", type=str, required=True
    )
    parser.add_argument(
        "--spanning_tree",
        help="Path to spanning tree binary to use",
        type=str,
        required=True,
    )
    parser.add_argument(
        "--data_files",
        help="Data files to use, one per node",
        type=str,
        nargs="+",
        required=True,
    )

    args = parser.parse_args()

    spanning_tree_args = [
        args.spanning_tree,
        "--nondaemon",
        "-p",
        str(SPANNING_TREE_PORT),
    ]
    print("Starting spanning_tree with args: " + " ".join(spanning_tree_args[1:]))
    spanning_tree_proc = subprocess.Popen(
        spanning_tree_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )

    vw_procs = []
    for index, data_file in enumerate(args.data_files):
        cmd_args = [
            args.vw,
            "--span_server",
            "localhost",
            "--total",
            str(len(args.data_files)),
            "--node",
            str(index),
            "--unique_id",
            "1234",
            "-d",
            data_file,
            "--span_server_port",
            str(SPANNING_TREE_PORT),
            "-q",
            "ab",
            "--passes",
            "1",
            "--holdout_off",
            "--predict_only_model",
            "--readable_model",
            "readable_model" + str(index) + ".txt",
        ]
        print("Starting VW with args: " + " ".join(cmd_args[1:]))
        vw_procs.append(
            subprocess.Popen(cmd_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        )

    for proc in vw_procs:
        return_code = proc.wait()
        print("VW succeeded")
        if return_code != 0:
            print("VW failed:")
            print("STDOUT: \n" + proc.stdout.read().decode("utf-8"))
            print("STDERR: \n" + proc.stderr.read().decode("utf-8"))
            spanning_tree_proc.kill()
            sys.exit(1)

    spanning_tree_proc.kill()

    readable_model_contents = []
    for index, _ in enumerate(args.data_files):
        with open("readable_model" + str(index) + ".txt") as f:
            readable_model_contents.append(f.read())

    if len(set(readable_model_contents)) != 1:
        print("Produced models are not the same")
        print("--------")
        print(readable_model_contents)
        sys.exit(1)
