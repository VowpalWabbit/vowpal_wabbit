#!/usr/bin/env bash
#
# Prepare a release commit: version, test reference files, changelog entry, pull request.
#
# These were the first four boxes on the release checklist, and they are entirely
# mechanical apart from the changelog prose. Doing them by hand is how you get a compare
# link pointing at a tag nobody cut, or a `git add -A` that sweeps a build directory into
# the commit.
#
#   utl/prepare-release.sh 9.11.8 --notes notes.md         # dry run
#   utl/prepare-release.sh 9.11.8 --notes notes.md --yes   # branch, commit, push, open PR
#   utl/prepare-release.sh 9.11.8 --yes                    # opens $EDITOR for the notes
#
# The changelog entry is a REQUIRED input, not a box to tick afterwards. Every release
# needs one, so asking for it up front is the honest interface: there is no way to run
# this and end up with a release that has nothing to say for itself. A skeleton with a
# TODO in it would just move the problem to whoever reviews the pull request.
#
# What this deliberately does not do is write that prose or push the tag. What changed,
# and whether to ship it, are the two judgements that stay with a person.
#
# Requires: gh (authenticated), git, python3.

set -euo pipefail

VERSION=""
NOTES_FILE=""
DRY_RUN=1

usage() {
  cat >&2 <<USAGE
Usage: $0 <version> [--notes <file>|-] [--yes]

  <version>        the release being prepared, e.g. 9.11.8
  --notes <file>   changelog entry body for this release; "-" reads stdin.
                   Omit it and \$EDITOR is opened, which needs a terminal.
  --yes            actually branch, commit, push and open the pull request.
                   Without it nothing is written.

The changelog entry is required. Write it as the body of the entry only: the
"## [version](compare link)" heading is generated, so do not include one.
USAGE
  exit 1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --notes)  NOTES_FILE="${2:-}"; [[ -z "$NOTES_FILE" ]] && usage; shift 2 ;;
    --notes=*) NOTES_FILE="${1#--notes=}"; shift ;;
    --yes)    DRY_RUN=0; shift ;;
    -h|--help) usage ;;
    -*)       echo "Unknown option: $1" >&2; usage ;;
    *)        [[ -n "$VERSION" ]] && usage; VERSION="$1"; shift ;;
  esac
done

[[ -z "$VERSION" ]] && usage

if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[0-9A-Za-z.-]+)?$ ]]; then
  echo "Refusing a version that is not semver-shaped: $VERSION" >&2
  exit 1
fi

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

# --- the changelog entry, up front ----------------------------------------------------
#
# Collected before anything is written, including in a dry run, so "I have nothing to say
# about this release" fails immediately rather than after the working tree has been
# edited. This is the one input that cannot be derived from the repository.

NOTES="$(mktemp)"
trap 'rm -f "$NOTES"' EXIT

say "Changelog entry"
if [[ -n "$NOTES_FILE" ]]; then
  if [[ "$NOTES_FILE" == "-" ]]; then
    note "reading from stdin"
    cat > "$NOTES"
  else
    [[ -r "$NOTES_FILE" ]] || { echo "Cannot read --notes file: $NOTES_FILE" >&2; exit 1; }
    note "reading from ${NOTES_FILE}"
    cat "$NOTES_FILE" > "$NOTES"
  fi
else
  # No --notes and no terminal means a script or CI invoked this. Guessing an entry would
  # be worse than stopping.
  if [[ ! -t 0 || ! -t 1 ]]; then
    echo "" >&2
    echo "No --notes given and no terminal to open an editor in." >&2
    echo "Every release needs a changelog entry, so this is a required input:" >&2
    echo "" >&2
    echo "  $0 ${VERSION} --notes <file>" >&2
    echo "  ... | $0 ${VERSION} --notes -" >&2
    exit 1
  fi
  EDITOR_CMD="${VISUAL:-${EDITOR:-}}"
  if [[ -z "$EDITOR_CMD" ]]; then
    echo "No --notes given and neither \$VISUAL nor \$EDITOR is set." >&2
    echo "Either set one or pass --notes <file>." >&2
    exit 1
  fi
  cat > "$NOTES" <<TEMPLATE
# Changelog entry for ${VERSION}. Lines starting with # are removed.
#
# What changed, in the terms a user of this release would care about. Cover everything
# since ${PREVIOUS}, which is the last RELEASED version -- not necessarily the last
# version bump. Do not add a "## ${VERSION}" heading; it is generated.
TEMPLATE
  note "opening ${EDITOR_CMD}"
  "$EDITOR_CMD" "$NOTES"
  sed -i '/^#/d' "$NOTES"
fi

# A heading in the body would nest under the generated one and break changelog_section.py.
if grep -q '^## ' "$NOTES"; then
  echo "The changelog entry contains its own '## ' heading." >&2
  echo "Pass the body only; the version heading and compare link are generated." >&2
  exit 1
fi
if [[ ! -s "$NOTES" ]] || ! grep -q '[^[:space:]]' "$NOTES"; then
  echo "The changelog entry is empty. A release with nothing to say for itself is a" >&2
  echo "release nobody can evaluate, so this is refused rather than defaulted." >&2
  exit 1
fi
note "$(grep -c '' < "$NOTES") lines, $(wc -w < "$NOTES") words"

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
  python3 - "$VERSION" "$PREVIOUS" "$UPSTREAM" "$NOTES" <<'PYINSERT'
import sys

version, previous, upstream, notes_path = sys.argv[1:5]
path = "CHANGELOG.md"

# Byte mode throughout: this file is CRLF, and rewriting it as LF turns a 20-line change
# into a 130-line diff.
data = open(path, "rb").read()
nl = b"\r\n" if b"\r\n" in data else b"\n"

anchor = f"## [{previous}](".encode()
if anchor not in data:
    sys.exit(f"Could not find the {previous} section to insert above.")

body = open(notes_path, "rb").read().replace(b"\r\n", b"\n").strip(b"\n")
heading = f"## [{version}](https://github.com/{upstream}/compare/{previous}...{version})"

entry = (heading.encode() + b"\n\n" + body + b"\n\n").replace(b"\n", nl)
open(path, "wb").write(data.replace(anchor, entry + anchor, 1))
PYINSERT
  note "entry added above the ${PREVIOUS} section"
  note "compare link: ${PREVIOUS}...${VERSION} (previous tag confirmed to exist)"
else
  note "[dry run] would insert the entry with a ${PREVIOUS}...${VERSION} compare link"
  note "[dry run] first line: $(head -1 "$NOTES")"
fi

# --- commit and PR --------------------------------------------------------------------

if [[ "$DRY_RUN" -eq 1 ]]; then
  say "Done (dry run)"
  note "Re-run with --yes to apply."
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

Version bump, the ${#REFS[@]} test reference files that embed the version string, and the
changelog entry for this release."

say "Pushing and opening the pull request"
git push -q -u origin "$BRANCH"
gh pr create --repo "$UPSTREAM" --base master --head "$BRANCH" \
  --title "chore: release ${VERSION}" \
  --body "Prepared by \`utl/prepare-release.sh\`.

- \`version.txt\`: ${PREVIOUS} -> ${VERSION}
- ${#REFS[@]} test reference files that embed the version string
- \`CHANGELOG.md\` entry with a ${PREVIOUS}...${VERSION} compare link (${PREVIOUS} confirmed tagged)

After merging, push the \`${VERSION}\` tag. That cuts the GitHub release and publishes to
PyPI, NuGet and Maven Central. Then run \`utl/post-release.sh ${VERSION} --yes\` for
Docker and vcpkg, and \`utl/verify-release.py ${VERSION}\` to confirm everything landed."

say "Done"
note "Review the pull request, get it merged, then push the ${VERSION} tag."
