import sys

import vowpalwabbit

# Known limitations:
# - Stdin cannot be used
# - vw.exe specifc args cannot be used: --args, --onethread


def main():
    vowpalwabbit.Workspace(arg_list=sys.argv[1:])


if __name__ == "__main__":
    main()
