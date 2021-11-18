import sys

from vowpalwabbit import pyvw

# Known limitations:
# - Stdin cannot be used
# - vw.exe specifc args cannot be used: --args, --onethread


def main():
    opts = sys.argv[1:]
    pyvw.vw(" ".join(opts))


if __name__ == "__main__":
    main()
