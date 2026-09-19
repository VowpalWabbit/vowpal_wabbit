---
name: Release checklist
about: Track every destination a release has to reach
title: 'Release X.Y.Z'
labels: 'Release'
assignees: ''

---

Release **X.Y.Z**.

Most package destinations now publish automatically when the tag is pushed, but not all of
them, and "published" is not the same as "installable". This checklist exists because 9.11.3
was version-bumped and merged but never tagged or packaged, so its security fixes reached
nobody and the gap went unnoticed for weeks.

Reference: [Release-Process wiki page](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Release-Process)

### Prepare

- [ ] `version.txt` bumped
- [ ] Test reference files under `test/` updated (they embed the version string in line 1)
- [ ] `CHANGELOG.md` entry covering **every** user-visible change since the last *released*
      version — not merely since the last version bump
- [ ] Compare link in the changelog points at a tag that actually exists
- [ ] CI green on master

### Tag and GitHub release

- [ ] Tag pushed (`X.Y.Z`, lightweight, matching previous tags)
- [ ] GitHub release published, with notes summarising the important changes

The tag is what triggers publishing, so the automated destinations below only fire for
workflow files that exist **at the tagged commit**. A publish job merged after the tag was
cut will not run for that tag.

### Published automatically by the tag

Confirm each job succeeded; they are not fire-and-forget.

- [ ] **NuGet** — `Publish to nuget.org` job in `.NET Nugets`
      ([docs](../../nuget/dotnet/RELEASE.md))
- [ ] **PyPI** — `Publish to PyPI` job in `Python`
      ([docs](../../python/RELEASE.md))
- [ ] **Java / Maven Central** — published automatically by the tag; confirm the
      `Publish to Maven Central` job succeeded and the artifacts appear at
      [central.sonatype.com](https://central.sonatype.com/search?smo=true&q=vw-jni)
      (see [`java/RELEASE.md`](../../java/RELEASE.md)). This is the only pipeline holding
      stored credentials, because Maven Central has no OIDC trusted publishing. The Central
      Portal token expires: if publishing starts failing with a 401, check that first

NuGet and PyPI use trusted publishing (OIDC), so they hold no credentials. If either fails
with an authorization error, the trusted publisher registration is the first thing to check:
it binds to the **workflow file name**, so renaming a workflow breaks publishing silently,
and the registration is not validated when saved — a typo only surfaces at publish time.

### Published automatically, but on its own schedule

- [ ] **npm (WASM)** — versions independently of VW. Bump `wasm/package.json`, push a
      `wasm_v<version>` tag, and the `Publish to npm` job handles the rest
      ([docs](../../wasm/developer_readme.md#release-on-npm))
- [ ] **Homebrew** — the Homebrew bot picks up the GitHub release by itself. Check
      [the formula](https://formulae.brew.sh/formula/vowpal-wabbit); only run
      `brew bump-formula-pr vowpal-wabbit --version=X.Y.Z` if the bot has not acted
- [ ] **conda-forge** — the autotick bot follows PyPI, so this cannot happen before PyPI
      does, and automerge lands its PR once CI is green. Confirm that actually happened on
      the [feedstock](https://github.com/conda-forge/vowpalwabbit-feedstock): automerge
      removes the "nobody merged it" failure, not the "CI never went green" one, and a PR
      stuck on flaky builds will sit there silently

### Still manual

- [ ] **vcpkg** — port update PR against microsoft/vcpkg (new version, new commit hash,
      regenerated patch)
- [ ] **Docker** — see [docker-images](https://github.com/vowpalwabbit/docker-images#release-steps)

### Verify what users actually get

A successful publish is not the same as a working package. npm 0.0.9 published cleanly and
was broken for every consumer. The automated jobs each install and import their own package
after publishing, but the rest are unchecked.

- [ ] `pip install vowpalwabbit==X.Y.Z` in a clean venv, then
      `python -c 'from vowpalwabbit import Workspace'`
- [ ] `npm install @vowpalwabbit/vowpalwabbit@<npm version>`, then
      `node -e "require('@vowpalwabbit/vowpalwabbit')"`
- [ ] PyPI, npm and NuGet all show the new version as latest

### Follow-ups

- [ ] Security advisories updated to name this version as the fixed version, and published
- [ ] Support table on the [Python wiki page](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Python) updated
- [ ] `reinforcement_learning` repo: bump the version in `ext_libs` and confirm CI passes
