#!/usr/bin/env python3

import re
import sys
import subprocess


def debug_print(msg):
    print(f"[{sys.argv[0]}] {msg}", file=sys.stderr)


git_shallow = subprocess.check_output(
    ["git", "rev-parse", "--is-shallow-repository"], text=True
).strip()
if git_shallow != "false":
    debug_print("Error: Git repository is shallow!")
    debug_print("To fix, run: 'git fetch --unshallow --tags --recurse-submodules=no'")
    sys.exit(1)

git_describe = subprocess.check_output(
    ["git", "describe", "--tags", "--first-parent", "--long"], text=True
).strip()
debug_print("Output of 'git describe' is: " + git_describe)

r = re.compile(
    r"^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:-(?P<tag>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?-(?P<commit>\d+)-g(?P<hash>[0-9a-fA-F]+)$"
)
m = r.match(git_describe)

major = m.group("major")
minor = m.group("minor")
patch = m.group("patch")
prerelease_tag = m.group("tag")
commit_count = m.group("commit")  # number of commits after latest git tag
hash = m.group("hash")
debug_print(f"Major: {major}")
debug_print(f"Minor: {minor}")
debug_print(f"Patch: {patch}")
debug_print(f"Prerelease tag: {prerelease_tag}")
debug_print(f"Commits after latest Git tag: {commit_count}")
debug_print(f"Git hash: {hash}")

if commit_count == "0":
    # Zero commits after tag means that this is the tagged commit
    # This is an official release, don't append the CI tag
    if prerelease_tag is None:
        print(f"{major}.{minor}.{patch}+{hash}")
    else:
        print(f"{major}.{minor}.{patch}-{prerelease_tag}+{hash}")
else:
    # Non-official build
    # Append a "ci.[number]" tag
    if prerelease_tag is None:
        # Increment the patch number so that this build is versioned after previous official release
        print(f"{major}.{minor}.{int(patch)+1}-ci.{commit_count}+{hash}")
    else:
        # The most recent official release has a pre-release tag
        # Longer tags are always versioned after shorter tags, so no need to increment version number
        print(f"{major}.{minor}.{patch}-{prerelease_tag}.ci.{commit_count}+{hash}")
