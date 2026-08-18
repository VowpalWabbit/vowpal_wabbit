#!/usr/bin/env bash
# Install apt packages, tolerating the transient stalls that have been hanging
# hosted Linux runners.
#
# The failure mode this exists for is a hang, not an error: apt blocks
# indefinitely reaching a package mirror, so a plain `apt-get install` never
# returns and the job burns its whole timeout. `timeout` converts that hang
# into a failure, which can then be retried.
#
# `timeout` must run *inside* sudo, not outside it. Run as the unprivileged
# user it cannot signal the root apt-get at all, and without -k it then waits
# forever for a child that never dies -- which is exactly the hang it was
# meant to break. As root, with -k, it escalates to SIGKILL and always
# returns.
#
# Usage: apt-install.sh <package> [package...]
set -euo pipefail

if [[ $# -eq 0 ]]; then
  echo "apt-install.sh: no packages given" >&2
  exit 2
fi

for attempt in 1 2 3; do
  # An index refresh that fails is survivable; the install below is not.
  sudo -E timeout -k 30 300 apt-get update -o Acquire::Retries=3 || true

  if sudo -E timeout -k 30 600 apt-get install -y -o Acquire::Retries=3 "$@"; then
    exit 0
  fi

  echo "::warning::apt-get install stalled or failed (attempt ${attempt}/3); retrying"
  sleep 15
done

echo "apt-install.sh: failed to install after 3 attempts: $*" >&2
exit 1
