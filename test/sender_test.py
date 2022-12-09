import sys
import argparse
import subprocess
import os

DAEMON_PORT = 26551

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--vw", help="Path to VW binary to use", type=str, required=True
    )
    parser.add_argument(
        "--input_file",
        help="Data files to use, one per node",
        type=str,
        required=True,
    )
    args = parser.parse_args()

    daemon_opts = [
        args.vw,
        "--daemon",
        "--foreground",
        "--num_children=1"
    ]

    print("Starting vw daemon with args: " + " ".join(daemon_opts[1:]))
    vw_daemon_proc = subprocess.Popen(
        daemon_opts, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )

    sender_opts = [
        args.vw,
        "--sendto",
        f"localhost",
        f"--data={args.input_file}",
        f"--predictions=sender_test.predict"
    ]
    print("Starting vw sender with args: " + " ".join(sender_opts[1:]))
    sender_proc = subprocess.Popen(
        sender_opts, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )

    return_code = sender_proc.wait()
    if sender_proc.stdout:
        print("Sender STDOUT: \n" + sender_proc.stdout.read().decode("utf-8"))
    if sender_proc.stderr:
        print("Sender STDERR: \n" + sender_proc.stderr.read().decode("utf-8"))

    if return_code != 0:
        print("VW failed")
        # Kill entire process group
        subprocess.run(['pkill', '-9', 'vw'], check=True)


    # Check if daemon failed.
    daemon_none_or_return_code = vw_daemon_proc.poll()
    if daemon_none_or_return_code is not None:
        if vw_daemon_proc.stdout:
            print("Daemon STDOUT: \n" + vw_daemon_proc.stdout.read().decode("utf-8"))
        if vw_daemon_proc.stderr:
            print("Daemon STDOUT: \n" + vw_daemon_proc.stderr.read().decode("utf-8"))
        sys.exit(1)

    # Kill entire process group
    subprocess.run(['pkill', '-9', 'vw'], check=True)
