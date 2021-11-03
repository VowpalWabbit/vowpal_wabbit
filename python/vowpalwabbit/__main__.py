import sys

from vowpalwabbit import pyvw


def main():
    opts = sys.argv[1:]
    pyvw.vw(" ".join(opts))


if __name__ == "__main__":
    main()
