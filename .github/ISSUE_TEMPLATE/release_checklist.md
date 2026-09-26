---
name: Release checklist
about: Track a release through the steps that still need a person
title: 'Release X.Y.Z'
labels: 'Release'
assignees: ''

---

Release **X.Y.Z**.

Most of this is three commands. What is left is the part that needs a judgement: what
changed, whether to ship it, and whether the thing that shipped works.

Reference: [Release-Process wiki page](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Release-Process)

### 1. Prepare

```sh
utl/prepare-release.sh X.Y.Z          # dry run
utl/prepare-release.sh X.Y.Z --yes    # branch, edit, commit, push, open the PR
```

Bumps `version.txt`, rewrites the test reference files that embed the version string,
inserts a changelog skeleton, and refuses to run if the previous version was never tagged
(which is what would make the compare link point at nothing).

- [ ] `utl/prepare-release.sh X.Y.Z --yes`
- [ ] **Write the changelog entry.** Cover everything since the last *released* version,
      which is not always the last version bump. CI fails while the skeleton `TODO:` is
      still there.
- [ ] Release PR merged, CI green on master

`lint.release-consistency` checks on every PR that the changelog has a section for
`version.txt`, that it is not still the skeleton, and that its compare link names a tag
that exists. Those used to be three boxes here.

### 2. Tag

- [ ] Push the tag: `git tag X.Y.Z && git push origin X.Y.Z`

That is the whole step. The tag creates the GitHub release (notes taken from
`CHANGELOG.md`) and starts publishing to PyPI, NuGet and Maven Central.

Publishing runs from the workflow files **as they exist at the tagged commit**. A publish
fix merged after the tag was cut does not apply to that tag.

### 3. The steps a tag cannot reach

```sh
utl/post-release.sh X.Y.Z --yes
```

Dispatches the Docker image build and opens the vcpkg port PR. Both live in other
repositories, so they run from a maintainer's own `gh` auth rather than a stored
cross-repository token. Needs the `workflow` scope; it says so and stops if it is missing.

- [ ] `utl/post-release.sh X.Y.Z --yes`
- [ ] vcpkg PR merged (a vcpkg maintainer has to review it)

### 4. The two that still need hands

- [ ] **Maven Central** — the tag uploads and validates, then stops at VALIDATED. Press
      Publish at [central.sonatype.com](https://central.sonatype.com/publishing/deployments).
      Deliberate: Central is the only destination with no undo. Once a release has gone
      through cleanly, set `autoPublish` to `true` in `java/pom.xml.in` and delete this box
      ([`java/RELEASE.md`](../../java/RELEASE.md))
- [ ] **npm (WASM)** — blocked on registering a trusted publisher for the `@vowpalwabbit`
      scope. Until that exists, publish by hand following
      [`wasm/developer_readme.md`](../../wasm/developer_readme.md#publishing-by-hand-while-the-scope-is-blocked).
      The npm package versions independently of VW; delete this box once the publisher is
      registered and a `wasm_v*` tag does it

### 5. Confirm it actually landed

```sh
utl/verify-release.py X.Y.Z --npm <npm version>
```

Asks every registry directly — GitHub, PyPI, NuGet (all five packages), Maven Central, npm,
conda-forge, Homebrew, vcpkg, Docker Hub — and exits non-zero if anything we publish
ourselves is missing. Destinations that arrive on somebody else's schedule are reported as
still propagating rather than failed.

- [ ] `utl/verify-release.py X.Y.Z --npm <npm version>` reports no misses

Re-run it a day later for the bot-driven ones. conda-forge follows PyPI and automerges once
its CI is green; Homebrew follows the GitHub release.

A publish that succeeds is not the same as a package that works — npm 0.0.9 published
cleanly and was broken for every consumer — so also, once:

- [ ] `pip install vowpalwabbit==X.Y.Z` in a clean venv, then
      `python -c 'from vowpalwabbit import Workspace'`

### 6. Follow-ups

- [ ] Security advisories updated to name this version as the fixed version, and published
- [ ] Support table on the [Python wiki page](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Python) updated
- [ ] `reinforcement_learning` repo: bump the version in `ext_libs` and confirm CI passes
