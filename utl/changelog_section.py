#!/usr/bin/env python3
"""Print one version's section of CHANGELOG.md, for use as GitHub release notes.

Keeping the changelog and the release notes as one text means they cannot drift, and it
turns "forgot to write a changelog entry" into a failed release job rather than a release
page that says nothing.

    utl/changelog_section.py 9.11.7

Exits non-zero if the version has no section, so CI stops rather than publishing a release
with empty notes.
"""

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CHANGELOG = REPO_ROOT / "CHANGELOG.md"

# "## [9.11.7](...compare/9.11.6...9.11.7)" or a bare "## 9.11.7".
HEADING = re.compile(r"^##\s+(?:\[(?P<bracketed>[^\]]+)\]\([^)]*\)|(?P<bare>\S+))\s*$")


def sections(text):
    """Yield (version, body) for each '## <version>' block, in file order."""
    current, buf = None, []
    for line in text.splitlines():
        match = HEADING.match(line)
        if match:
            if current is not None:
                yield current, "\n".join(buf).strip()
            current = match.group("bracketed") or match.group("bare")
            buf = []
        elif current is not None:
            buf.append(line)
    if current is not None:
        yield current, "\n".join(buf).strip()


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("version")
    ap.add_argument("--changelog", default=str(CHANGELOG))
    args = ap.parse_args()

    text = Path(args.changelog).read_text(encoding="utf-8")
    found = dict(sections(text))

    body = found.get(args.version)
    if body is None:
        print(f"No CHANGELOG.md section for {args.version}.", file=sys.stderr)
        known = [v for v in found if v.lower() != "changelog"][:5]
        if known:
            print(f"Sections present: {', '.join(known)}", file=sys.stderr)
        return 1
    if not body:
        print(f"CHANGELOG.md section for {args.version} is empty.", file=sys.stderr)
        return 1

    print(body)
    return 0


if __name__ == "__main__":
    sys.exit(main())
