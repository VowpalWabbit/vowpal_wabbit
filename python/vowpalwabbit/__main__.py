import sys

from vowpalwabbit import pyvw


def main():
    opts = sys.argv[1:]
    pyvw.vw(" ".join(opts), allow_stdin=True)


if __name__ == "__main__":
    main()
