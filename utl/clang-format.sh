#!/usr/bin/env bash

# Set environment variable GH_WORKFLOW_LOGGING to output logging that Azure pipelines will interpret as a warning.
# Set environment variable WARNING_AS_ERROR to make the script exit with a non-zero code if issues are found.

if [[ "$1" != "check" && "$1" != "fix" && "$1" != "docker" ]]; then
    echo "Usage: \"$0 check\" or \"$0 fix\" or \"$0 docker check\" or \"$0 docker fix\""
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$SCRIPT_DIR/../"
cd "$REPO_DIR"

# For the docker commands, re-run this script inside docker container
# The ubuntu:22.04 image should have clang-format version 10, which matches what runs in Github Actions
if [[ "$1" == "docker" ]]; then
    DOCKER_CMD="echo 'Installing clang-format in docker container...'; apt update -qq; apt install -qq -y clang-format; cd /reinforcement_learning; ./utl/clang-format.sh ${@:2}"
    if command -v podman &> /dev/null; then
        # podman supports --env-host for forwarding all host environment variables
        podman run -it --env-host -v "$REPO_DIR:/reinforcement_learning" ubuntu:22.04 /bin/bash -c "$DOCKER_CMD"
    elif command -v docker &> /dev/null; then
        docker run -it -v "$REPO_DIR:/reinforcement_learning" ubuntu:22.04 /bin/bash -c "$DOCKER_CMD"
    else
        echo "You need to install Docker (or Podman) first to use this script in docker mode"
    fi
    exit
fi

# Otherwise, actually do the formatting check/fix
echo "Using clang-format version:"
clang-format --version

for FILE in $(find . -type f -not -path './ext_libs/*' -not -path './cs/cli/*' -not -path '*/vcpkg_installed/*' \( -name '*.cc' -o -name "*.h" \) ); do
    if [[ "$1" == "check" ]]; then
        clang-format --dry-run --Werror $FILE
        if [ $? -ne 0 ]; then
            ISSUE_FOUND="true"
            if [[ -v GH_WORKFLOW_LOGGING ]]; then
                echo "::warning:: Formatting issues found in $FILE"
            fi
        fi
    fi

    if [[ "$1" == "fix" ]]; then
        clang-format -i $FILE;
    fi
done

if [[ -v ISSUE_FOUND ]]; then
    echo -e "\nFormatting issues found:\n\tRun \"$0 fix\" or \"$0 docker fix\""
    if [[ -v WARNING_AS_ERROR ]]; then
        echo -e "\nTreating as failure because WARNING_AS_ERROR was set"
        exit 1
    fi
fi
