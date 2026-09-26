#!/usr/bin/env python3
"""Ask every package registry whether a release actually arrived.

The release checklist used to say "check NuGet", "check PyPI", "check conda-forge" and
so on: eleven boxes, each meaning open a website and squint at a version number. Nobody
does that reliably, which is how 9.11.3 was bumped and merged but never packaged, and how
Maven Central sat at 9.9.0 for over a year without anyone noticing.

Every one of those destinations has a public read API. This asks all of them at once.

    utl/verify-release.py 9.11.7
    utl/verify-release.py 9.11.7 --npm 0.0.10

Exit status is 0 only when every destination that should have the version has it, so this
works as a CI gate as well as a thing a human runs.

No credentials and no writes: it is safe to run at any time, including against a release
that has not happened yet, to see how far along it is.
"""

import argparse
import concurrent.futures
import json
import sys
import urllib.error
import urllib.request

TIMEOUT = 30
UA = {"User-Agent": "vw-verify-release"}

# Registry states. "pending" is not a failure: bots and mirrors take their own time.
LIVE, MISSING, PENDING, ERROR, SKIPPED = "live", "missing", "pending", "error", "skipped"


def _get(url):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
        return resp.status, resp.read()


def _json(url):
    status, body = _get(url)
    return status, json.loads(body)


def _exists(url):
    """True if the URL is fetchable, False on 404. Anything else is an error."""
    try:
        status, _ = _get(url)
        return status == 200
    except urllib.error.HTTPError as exc:
        if exc.code == 404:
            return False
        raise


def check_pypi(version, _npm):
    ok = _exists(f"https://pypi.org/pypi/vowpalwabbit/{version}/json")
    return (LIVE, "") if ok else (MISSING, "not on pypi.org")


def check_nuget(version, _npm):
    # Five packages ship together; a partial publish is the interesting failure, so report
    # each rather than stopping at the first hit.
    ids = [
        "vowpalwabbit",
        "vowpalwabbit.runtime.win-x64",
        "vowpalwabbit.runtime.linux-x64",
        "vowpalwabbit.runtime.osx-x64",
        "vowpalwabbit.runtime.osx-arm64",
    ]
    missing = []
    for pkg in ids:
        _, doc = _json(f"https://api.nuget.org/v3-flatcontainer/{pkg}/index.json")
        if version not in doc.get("versions", []):
            missing.append(pkg)
    if not missing:
        return LIVE, "all 5 packages"
    return MISSING, f"{len(missing)}/5 missing: {', '.join(missing)}"


def check_maven(version, _npm):
    base = "https://repo1.maven.org/maven2/com/github/vowpalwabbit/vw-jni"
    ok = _exists(f"{base}/{version}/vw-jni-{version}.jar")
    if ok:
        return LIVE, ""
    # Distinguish "this version is missing" from "nothing was ever published", because the
    # second means the pipeline has never worked and the first means one release failed.
    try:
        _, body = _get(f"{base}/maven-metadata.xml")
        text = body.decode("utf-8", "replace")
        latest = text.split("<release>")[1].split("</release>")[0] if "<release>" in text else "?"
        return MISSING, f"latest on Central is {latest}"
    except Exception:
        return MISSING, "nothing published under this groupId"


def check_npm(_version, npm_version):
    if not npm_version:
        return SKIPPED, "versions independently; pass --npm to check"
    _, doc = _json("https://registry.npmjs.org/@vowpalwabbit/vowpalwabbit")
    if npm_version in doc.get("versions", {}):
        return LIVE, f"@vowpalwabbit/vowpalwabbit {npm_version}"
    latest = doc.get("dist-tags", {}).get("latest", "?")
    return MISSING, f"latest on npm is {latest}"


def check_conda(version, _npm):
    _, doc = _json("https://api.anaconda.org/package/conda-forge/vowpalwabbit")
    if version in doc.get("versions", []):
        return LIVE, ""
    return PENDING, f"feedstock latest is {doc.get('latest_version', '?')}"


def check_homebrew(version, _npm):
    _, doc = _json("https://formulae.brew.sh/api/formula/vowpal-wabbit.json")
    stable = doc.get("versions", {}).get("stable")
    if stable == version:
        return LIVE, ""
    return PENDING, f"formula is at {stable}"


def check_vcpkg(version, _npm):
    url = "https://raw.githubusercontent.com/microsoft/vcpkg/master/ports/vowpal-wabbit/vcpkg.json"
    _, doc = _json(url)
    port = doc.get("version")
    if port == version:
        return LIVE, ""
    return PENDING, f"port is at {port}"


def check_docker(version, _npm):
    url = f"https://hub.docker.com/v2/repositories/vowpalwabbit/vw-rel-alpine/tags/{version}"
    return (LIVE, "") if _exists(url) else (PENDING, "no such tag on Docker Hub")


def check_github(version, _npm):
    api = f"https://api.github.com/repos/VowpalWabbit/vowpal_wabbit/releases/tags/{version}"
    try:
        _, doc = _json(api)
        return (LIVE, "draft" if doc.get("draft") else "")
    except urllib.error.HTTPError as exc:
        if exc.code == 404:
            tag = f"https://api.github.com/repos/VowpalWabbit/vowpal_wabbit/git/ref/tags/{version}"
            if _exists(tag):
                # This is the 9.11.3 failure exactly: a tag with no release behind it.
                return MISSING, "tag exists but no GitHub release"
            return MISSING, "no tag and no release"
        raise


# Ordered by how early in the release they should appear, so a partial run reads top-down.
CHECKS = [
    ("GitHub release", check_github),
    ("PyPI", check_pypi),
    ("NuGet", check_nuget),
    ("Maven Central", check_maven),
    ("npm (WASM)", check_npm),
    ("conda-forge", check_conda),
    ("Homebrew", check_homebrew),
    ("vcpkg", check_vcpkg),
    ("Docker", check_docker),
]

# conda-forge, Homebrew, vcpkg and Docker arrive on somebody else's schedule, so their
# absence is news rather than failure. Everything else is published by our own tag.
SOFT = {"conda-forge", "Homebrew", "vcpkg", "Docker", "npm (WASM)"}

MARK = {LIVE: "OK  ", MISSING: "MISS", PENDING: "WAIT", ERROR: "ERR ", SKIPPED: "--  "}


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("version", help="VW version, e.g. 9.11.7")
    ap.add_argument("--npm", metavar="V", help="npm package version (versions independently)")
    ap.add_argument("--strict", action="store_true",
                    help="also fail on destinations that publish on their own schedule")
    args = ap.parse_args()

    def run(item):
        name, fn = item
        try:
            return name, fn(args.version, args.npm)
        except Exception as exc:  # network, JSON, anything: report, do not abort the rest
            return name, (ERROR, f"{type(exc).__name__}: {exc}")

    with concurrent.futures.ThreadPoolExecutor(max_workers=len(CHECKS)) as pool:
        results = dict(pool.map(run, CHECKS))

    print(f"\nVowpal Wabbit {args.version}"
          + (f"  (npm {args.npm})" if args.npm else "") + "\n")
    width = max(len(n) for n, _ in CHECKS)
    for name, _ in CHECKS:
        state, note = results[name]
        print(f"  {MARK[state]}  {name.ljust(width)}  {note}")

    hard = [n for n, _ in CHECKS
            if results[n][0] in (MISSING, ERROR) and (args.strict or n not in SOFT)]
    waiting = [n for n, _ in CHECKS if results[n][0] == PENDING]

    print()
    if hard:
        print(f"Not released: {', '.join(hard)}")
        return 1
    if waiting:
        print(f"Released. Still propagating: {', '.join(waiting)}")
    else:
        print("Released everywhere.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
