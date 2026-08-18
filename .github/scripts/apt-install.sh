#!/usr/bin/env bash
# Install apt packages, tolerating the transient stalls that have been hanging
# hosted Linux runners.
#
# The failure mode this exists for is a hang, not an error: apt blocks
# indefinitely reaching a package mirror, so a plain `apt-get install` never
# returns and the job burns its whole timeout. `timeout` converts that hang
# into a failure, which can then be retried.
#
# Usage: apt-install.sh <package> [package...]
set -euo pipefail

if [[ $# -eq 0 ]]; then
  echo "apt-install.sh: no packages given" >&2
  exit 2
fi

for attempt in 1 2 3; do
  # An index refresh that fails is survivable; the install below is not.
  timeout 300 sudo -E apt-get update -o Acquire::Retries=3 || true

  if timeout 600 sudo -E apt-get install -y -o Acquire::Retries=3 "$@"; then
    exit 0
  fi

  echo "::warning::apt-get install stalled or failed (attempt ${attempt}/3); retrying"
  sleep 15
done

echo "apt-install.sh: failed to install after 3 attempts: $*" >&2
exit 1
