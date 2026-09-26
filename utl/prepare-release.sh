#!/usr/bin/env bash
#
# Prepare a release commit: version, test reference files, changelog skeleton, pull request.
#
# These were the first four boxes on the release checklist, and they are entirely
# mechanical apart from the changelog prose. Doing them by hand is how you get a compare
# link pointing at a tag nobody cut, or a `git add -A` that sweeps a build directory into
# the commit.
#
#   utl/prepare-release.sh 9.11.8            # show what would happen, change nothing
#   utl/prepare-release.sh 9.11.8 --yes      # branch, edit, commit, push, open the PR
#
# It deliberately does NOT write the changelog prose or push the tag. What changed and
# whether to ship it are the two judgements that should stay with a person.
#
# Requires: gh (authenticated), git, python3.

set -euo pipefail

VERSION="${1:-}"
APPLY="${2:-}"

if [[ -z "$VERSION" ]]; then
  echo "Usage: $0 <version> [--yes]" >&2
  echo "  e.g. $0 9.11.8 --yes" >&2
  exit 1
fi

if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[0-9A-Za-z.-]+)?$ ]]; then
  echo "Refusing a version that is not semver-shaped: $VERSION" >&2
  exit 1
fi

DRY_RUN=1
[[ "$APPLY" == "--yes" ]] && DRY_RUN=0

UPSTREAM="VowpalWabbit/vowpal_wabbit"
BRANCH="chore/release-${VERSION}"

say()  { printf '\n\033[1m==> %s\033[0m\n' "$*"; }
note() { printf '    %s\n' "$*"; }

cd "$(git rev-parse --show-toplevel)"

PREVIOUS="$(tr -d '[:space:]' < version.txt)"

say "Preparing ${PREVIOUS} -> ${VERSION}"

if [[ "$PREVIOUS" == "$VERSION" ]]; then
  echo "version.txt already says ${VERSION}." >&2
  exit 1
fi

# The changelog's compare link points at the previous version, and that only works if the
# previous version was actually tagged. 9.11.3 was bumped and merged but never tagged, so
# the next release's compare link pointed at nothing. Check rather than assume.
say "Checking the previous version was really released"
if git ls-remote --tags "https://github.com/${UPSTREAM}.git" "refs/tags/${PREVIOUS}" \
   | grep -q "refs/tags/${PREVIOUS}"; then
  note "tag ${PREVIOUS} exists; compare link will resolve"
else
  echo "" >&2
  echo "version.txt says ${PREVIOUS} but there is no ${PREVIOUS} tag upstream." >&2
  echo "That version was bumped but never released, so a ${PREVIOUS}...${VERSION} compare" >&2
  echo "link would point at nothing. Use the last version that was really tagged:" >&2
  git ls-remote --tags "https://github.com/${UPSTREAM}.git" \
    | sed -n 's|.*refs/tags/\([0-9][0-9.]*\)$|  \1|p' | sort -V | tail -5 >&2
  exit 1
fi

if [[ "$DRY_RUN" -eq 1 ]]; then
  say "DRY RUN -- nothing will be written, committed, pushed or opened."
fi

# --- the mechanical edits -------------------------------------------------------------

say "version.txt"
note "${PREVIOUS} -> ${VERSION}"
[[ "$DRY_RUN" -eq 0 ]] && echo "$VERSION" > version.txt

# Readable models write the version into their first line, so every reference file that
# contains one has to move with the release.
say "Test reference files"
mapfile -t REFS < <(grep -rl "^Version ${PREVIOUS}\$" test/ || true)
note "${#REFS[@]} files contain \"Version ${PREVIOUS}\""
if [[ "${#REFS[@]}" -gt 0 && "$DRY_RUN" -eq 0 ]]; then
  # In place, first line only, so a file that merely mentions the version elsewhere is
  # left alone.
  sed -i "s/^Version ${PREVIOUS}\$/Version ${VERSION}/" "${REFS[@]}"
fi

say "CHANGELOG.md"
if [[ "$DRY_RUN" -eq 0 ]]; then
  python3 - "$VERSION" "$PREVIOUS" "$UPSTREAM" <<'PY'
import sys
version, previous, upstream = sys.argv[1], sys.argv[2], sys.argv[3]
path = "CHANGELOG.md"
# Byte mode: this file is CRLF, and rewriting it as LF makes a 130-line diff out of a
# 20-line change.
data = open(path, "rb").read()
nl = b"\r\n" if b"\r\n" in data else b"\n"
anchor = f"## [{previous}](".encode()
if anchor not in data:
    sys.exit(f"Could not find the {previous} section to insert above.")
entry = (
    f"## [{version}](https://github.com/{upstream}/compare/{previous}...{version})\n"
    "\n"
    "TODO: what changed, in the terms a user of this release would care about. Cover\n"
    "everything since the last *released* version, which is not always the last version\n"
    "bump. Delete this line.\n"
    "\n"
).replace("\n", nl.decode()).encode()
open(path, "wb").write(data.replace(anchor, entry + anchor, 1))
PY
  note "skeleton entry added above the ${PREVIOUS} section"
  note "compare link: ${PREVIOUS}...${VERSION} (previous tag confirmed to exist)"
else
  note "[dry run] would insert a skeleton entry with a ${PREVIOUS}...${VERSION} compare link"
fi

# --- commit and PR --------------------------------------------------------------------

if [[ "$DRY_RUN" -eq 1 ]]; then
  say "Done (dry run)"
  note "Re-run with --yes to apply. You will still have to write the changelog entry."
  exit 0
fi

say "Committing"
git checkout -q -b "$BRANCH"
# `git add -u` and not `git add -A`: a working tree with a build directory in it will
# otherwise put several hundred megabytes of object files in the release commit, and the
# push hangs rather than failing, which is a confusing way to find out.
git add -u
FILES="$(git diff --cached --name-only | wc -l)"
note "${FILES} tracked files staged (untracked files deliberately ignored)"
git commit -q -m "chore: release ${VERSION}

Version bump and the ${#REFS[@]} test reference files that embed the version string."

say "Pushing and opening the pull request"
git push -q -u origin "$BRANCH"
gh pr create --repo "$UPSTREAM" --base master --head "$BRANCH" \
  --title "chore: release ${VERSION}" \
  --body "Prepared by \`utl/prepare-release.sh\`.

- \`version.txt\`: ${PREVIOUS} -> ${VERSION}
- ${#REFS[@]} test reference files that embed the version string
- \`CHANGELOG.md\` skeleton with a ${PREVIOUS}...${VERSION} compare link (${PREVIOUS} confirmed tagged)

**The changelog entry still needs writing** before this merges.

After merging, push the \`${VERSION}\` tag. That cuts the GitHub release and publishes to
PyPI, NuGet and Maven Central. Then run \`utl/post-release.sh ${VERSION} --yes\` for
Docker and vcpkg, and \`utl/verify-release.py ${VERSION}\` to confirm everything landed."

say "Done"
note "Write the changelog entry, get it merged, then push the ${VERSION} tag."
