import sys

import vowpalwabbit

# Known limitations:
# - Stdin cannot be used
# - vw.exe specifc args cannot be used: --args, --onethread


def main():
    opts = sys.argv[1:]
    vowpalwabbit.Workspace(" ".join(opts))


if __name__ == "__main__":
    main()
