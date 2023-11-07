import argparse
import subprocess
import time
import sys

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--vw", help="Path to VW binary to use", type=str, required=True
    )
    parser.add_argument(
        "--active_interactor",
        help="Path to python file to use",
        type=str,
        required=True,
    )
    parser.add_argument(
        "--unlabeled_data",
        help="Unlabeled data to use",
        type=str,
        required=True,
    )
    parser.add_argument(
        "--labels",
        help="Labels for unlabeled data",
        type=str,
        required=True,
    )
    parser.add_argument(
        "--port",
        help="Port to use for active",
        type=str,
        required=True,
    )
    args = parser.parse_args()

    active_vw_args = [
        args.vw,
        "--active",
        "--port",
        args.port,
    ]
    print("Starting vw with args: " + " ".join(active_vw_args[1:]))
    active_vw_proc = subprocess.Popen(
        active_vw_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    # sleep for a bit to let vw start up
    time.sleep(0.1)

    cmd_args = [
        sys.executable,
        args.active_interactor,
        "localhost",
        args.port,
        args.unlabeled_data,
    ]

    active_interactor_proc = subprocess.Popen(
        cmd_args, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE
    )
    with open(args.labels, "r") as f:
        labels = f.read()
        stdout_data, stderr_data = active_interactor_proc.communicate(
            input=labels.encode("utf-8")
        )
        print("active_interactor stdout: \n" + stdout_data.decode("utf-8"))
        print("active_interactor stderr: \n" + stderr_data.decode("utf-8"))

    active_vw_proc.wait()
    if active_vw_proc.stdout is not None:
        print("vw active stdout: \n" + active_vw_proc.stdout.read().decode("utf-8"))

    if active_vw_proc.stderr is not None:
        print("vw active stderr: \n" + active_vw_proc.stderr.read().decode("utf-8"))
