#!/usr/bin/env python3
import socket
import argparse
from typing import Callable, ContextManager, Optional, TextIO
import contextlib

TEXT_ENCODING = "utf-8"
NEWLINE_CHAR_BYTE = b"\n"[0]


def recvall(s: socket.socket, bufsize: int) -> str:
    received_buffer = s.recv(bufsize)
    received_num_bytes = len(received_buffer)
    while received_num_bytes > 0 and len(received_buffer) < bufsize:
        if received_buffer[-1] == NEWLINE_CHAR_BYTE:
            break
        tmp_buffer = s.recv(bufsize)
        received_num_bytes = len(tmp_buffer)
        received_buffer = received_buffer + tmp_buffer
    return received_buffer.decode(TEXT_ENCODING)


def _get_getch_impl_unix() -> Optional[Callable[[], str]]:
    try:
        import sys  # type: ignore
        import tty  # type: ignore
        import termios  # type: ignore
    except ImportError:
        return None

    def _getch():
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        ch = None
        try:
            tty.setraw(fd)
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
            if ch is not None and ord(ch) == 3:
                raise KeyboardInterrupt
            return ch

    return _getch


def _get_getch_impl_win() -> Optional[Callable[[], str]]:
    try:
        # try Windows API
        import msvcrt  # type: ignore
    except ImportError:
        return None

    def _getch():
        ch = msvcrt.getch
        if ord(ch) == 3:
            raise KeyboardInterrupt
        return ch

    return _getch


# The MacOS Carbon API was removed in 10.15, however the
# standard unix approach should work and so should be tried first.
def _get_getch_impl_macos() -> Optional[Callable[[], str]]:

    try:
        # try OS/X API
        import Carbon  # type: ignore

        # unix doesn't have this
        Carbon.Evt
    except:
        return None

    def _getch():
        if Carbon.Evt.EventAvail(0x0008)[0] == 0:  # 0x0008 is the keyDownMask
            return ""
        else:
            (what, msg, when, where, mod) = Carbon.Evt.GetNextEvent(0x0008)[1]
            ch = chr(msg & 0x000000FF)
            if ord(ch) == 3:
                raise KeyboardInterrupt
            return ch

    return _getch


def try_find_getch():
    macos_getch = _get_getch_impl_macos()
    if macos_getch is not None:
        return macos_getch

    win_getch = _get_getch_impl_win()
    if win_getch is not None:
        return win_getch

    unix_getch = _get_getch_impl_unix()
    if unix_getch is not None:
        return unix_getch

    raise Exception(
        "Could not find a getch implementation. --keypress cannot be used without it."
    )


def check_file_exists(file_path: str, file_mode: str) -> None:
    with open(file_path, file_mode) as _:
        pass


def get_label(
    example: str,
    minus1: bool,
    i: int,
    tag: str,
    pred: float,
    input_fn: Callable[[str], str],
) -> Optional[str]:
    print(
        'Request for example {}: tag="{}", prediction={}: {}'.format(
            i, tag, pred, example
        )
    )
    while True:
        label = input_fn("Provide? [0/1/skip]: ")
        if label == "1":
            break
        if label == "0":
            if minus1:
                label = "-1"
            break
        if 0 < len(label) <= len("skip") and "skip".startswith(label):
            label = None
            break
    return label


def send_example(sock: socket.socket, example_line: str) -> None:
    # We must ensure there is a newline present at the end of the line.
    line = example_line.strip() + "\n"
    sock.sendall(line.encode(TEXT_ENCODING))


def active_process_unlabeled_dataset(
    sock: socket.socket,
    unlabeled: TextIO,
    input_fn: Callable[[str], str],
    minus1: bool,
    verbose=False,
    human: Optional[TextIO] = None,
    output: Optional[TextIO] = None,
):
    for i, line in enumerate(unlabeled):
        human_string = line
        if human is not None:
            try:
                human_string = next(human)
            except:
                print("Warning: out of lines in human data, reverting to vw strings")
                human_string = line
        send_example(sock, line)
        response = recvall(sock, 256)
        responselist = response.split(" ")
        if len(responselist) == 2:
            # VW does not care about this label
            if verbose:
                print(
                    'Importance omitted from response for example {}, "{}", skipping...'.format(
                        i, response.strip()
                    )
                )
            continue
        prediction, tag, importance = responselist
        if verbose:
            print(
                "Example {} reponse: prediction={}, tag={}, importance={}".format(
                    i, prediction, tag, importance
                )
            )
        try:
            imp = float(importance)
        except:
            continue
        label = get_label(
            human_string if verbose else "",
            minus1,
            i,
            tag,
            float(prediction),
            input_fn,
        )
        if label is None:
            continue
        front, rest = line.split("|", 1)
        if tag == "":
            tag = "'empty"
        labeled_example = "{} {} {} |{}".format(label, imp, tag, rest)
        send_example(sock, labeled_example)
        if output:
            output.write(labeled_example)
            output.flush()
        recvall(sock, 256)


# This is normally only available in 3.7+, bring it here for 3.6
class backported_nullcontext(contextlib.AbstractContextManager):
    def __enter__(self):
        return None

    def __exit__(self, *excinfo):
        pass


def open_if_not_none(file_path: str, file_mode: str) -> ContextManager:
    if file_path is not None:
        return open(file_path, file_mode)
    else:
        return backported_nullcontext()


def main():
    parser = argparse.ArgumentParser(
        description="interact with VW in active learning mode"
    )
    parser.add_argument("-s", "--seed", help="seed labeled dataset")
    parser.add_argument("-o", "--output", help="output file")
    parser.add_argument(
        "-u",
        "--human",
        help="human readable examples in this file (same # of lines as unlabeled); use with -v",
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="show example (in addition to tag)"
    )
    parser.add_argument("-m", "--minus1", action="store_true", help="interpret 0 as -1")
    parser.add_argument(
        "-k",
        "--keypress",
        action="store_true",
        help="don't require 'Enter' after keypresses",
    )
    parser.add_argument("host", help="the machine VW is running on")
    parser.add_argument("port", type=int, help="the port VW is listening on")
    parser.add_argument("unlabeled_dataset", help="file with unlabeled data")
    args = parser.parse_args()

    input_fn = input
    if args.keypress:
        getch = try_find_getch()

        def raw_input_keypress(str: str) -> str:
            print(str, end="")
            return getch()

        input_fn = raw_input_keypress

    check_file_exists(args.unlabeled_dataset, "r")
    if args.seed is not None:
        check_file_exists(args.seed, "r")
    if args.human is not None:
        check_file_exists(args.human, "r")
    if args.output is not None:
        check_file_exists(args.output, "w")

    # Create a socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        # Connect to server and perform handshake
        print("Connecting to {}:{}...".format(args.host, args.port))
        sock.connect((args.host, args.port))
        print("Done")

        # Send seed dataset
        if args.seed is not None:
            print("Seeding vw...")
            with open(args.seed, "r") as seed_file:
                for line in seed_file:
                    send_example(sock, line)
                    sock.sendall(line.encode(TEXT_ENCODING))
                    recvall(sock, 256)
            print("Done")

        # Send unlabeled dataset
        print("Sending unlabeled examples...")
        with open(args.unlabeled_dataset, "r") as unlabeled:
            with open_if_not_none(args.human, "r") as human:
                with open_if_not_none(args.output, "w") as output:
                    active_process_unlabeled_dataset(
                        sock=sock,
                        unlabeled=unlabeled,
                        input_fn=input_fn,
                        minus1=args.minus1,
                        verbose=args.verbose,
                        human=human,
                        output=output,
                    )


if __name__ == "__main__":
    main()
