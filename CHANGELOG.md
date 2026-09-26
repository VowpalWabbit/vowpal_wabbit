# Changelog

All notable changes to Vowpal Wabbit are documented in this file. For changes
prior to this file's creation, see [GitHub Releases](https://github.com/VowpalWabbit/vowpal_wabbit/releases).

## [9.11.7](https://github.com/VowpalWabbit/vowpal_wabbit/compare/9.11.6...9.11.7)

No library code changes. This release exists to exercise the Maven Central pipeline, which
was built for 9.11.6 but has never run against the live service.

Maven Central is at 9.11.1, pushed by hand in March 2026, and has received nothing since;
before that it sat at 9.9.0 from the OSSRH shutdown on 2025-06-30. The replacement
publishes through the Sonatype Central Portal and is wired to the release tag, but an
untested credentialed pipeline is not the same as a working one, and Central is the only
destination here that cannot be undone: a published version can be superseded, never
withdrawn. So this release uploads and stops at VALIDATED, and going live is one
deliberate click.

npm is unaffected by this tag. The WASM package versions independently and publishes from
its own `wasm_v*` tag; that pipeline is currently blocked on registering a trusted
publisher for the `@vowpalwabbit` scope, so it is being published by hand in the meantime.
See `wasm/developer_readme.md`.

### Changed

- Maven Central deployments stop at VALIDATED rather than publishing automatically, until
  the pipeline has completed one clean release (`java/pom.xml.in`)

### Added

- `utl/post-release.sh`, which dispatches the Docker image build and opens the vcpkg port
  pull request, the two destinations that live in other repositories (#4962, #4963)
- Documented manual npm publishing steps for as long as the scope is blocked

## [9.11.6](https://github.com/VowpalWabbit/vowpal_wabbit/compare/9.11.5...9.11.6)

Identical in content to 9.11.5, which failed to publish.

9.11.5 was the first release meant to reach the package registries automatically, and both
publish jobs failed: the .NET job ran `git fetch --unshallow` against an already complete
checkout and aborted before uploading, and the PyPI trusted publisher had been registered
against `python_wheels.yaml` rather than `python_wheels.yml`, so the OIDC exchange found no
matching publisher. Nothing was uploaded and nothing was left partially published; both
failures were in the publish step itself.

Publishing runs from the workflow file as it exists at the tagged commit, so neither fix
could apply retroactively to the 9.11.5 tag. This release carries them.

**If you install Vowpal Wabbit from pip or NuGet, this is the first release containing the
security fixes from 9.11.3 and 9.11.4.** See those entries below for what they address.

### Fixed

- Remove the `--unshallow` step from the NuGet publish job, which fails on the complete
  checkout that job uses (#4959)

## [9.11.5](https://github.com/VowpalWabbit/vowpal_wabbit/compare/9.11.4...9.11.5)

A packaging release. It contains no library code changes: its purpose is to reach the
package registries that 9.11.4 did not.

9.11.4 was tagged and released on GitHub, but the .NET, Python and WASM packages were
still published by hand, and were not. Those packages had drifted badly as a result --
NuGet was last published at 9.3.0, PyPI at 9.11.2 -- so the security fixes in 9.11.3 and
9.11.4 were unavailable to anyone installing from a package manager. Publishing now
happens automatically when a release tag is pushed, so this release is the first to carry
those fixes to package users.

**If you install Vowpal Wabbit from pip or NuGet, this is the first release containing the
security fixes from 9.11.3 and 9.11.4.** See those entries below for what they address.

### Changed

- Publish the .NET packages to nuget.org automatically on a release tag, using trusted
  publishing rather than a stored API key (#4953)
- Publish the npm WASM package automatically on a `wasm_v*` tag, and always run the
  TypeScript build before publishing. 0.0.9 shipped without its transpiled entry points and
  was unusable from Node (#4951, #4954)
- Publish the Python wheels and source distribution to PyPI automatically on a release tag.
  The macOS ARM wheels no longer need building by hand; CI has produced them since GitHub
  added Apple Silicon runners (#4955)

## [9.11.4](https://github.com/VowpalWabbit/vowpal_wabbit/compare/9.11.2...9.11.4)

Security patch release addressing three further model-loading vulnerabilities and
completing the slates DSJSON fix started in 9.11.3.

**9.11.3 was never published** -- its version bump landed but no tag, GitHub release or
package was produced, so no artifact containing its fixes was ever available. 9.11.4
therefore supersedes it and carries everything from it, including the two advisories
listed under 9.11.3 below (GHSA-x3cx-p52g-p5q7 and GHSA-c8v3-p4fg-v3pm). Anyone still on
9.11.2 or earlier should upgrade directly to 9.11.4.

All four are reachable through ordinary use -- loading a model with `-i`, or parsing
slates DSJSON input. In every case an attacker-controlled length or count taken from
the input file was guarded only by an `assert`, which `-DNDEBUG` removes from `Release`
and `RelWithDebInfo` builds, so these affected optimized builds and not just sanitizer
ones.

### Security

- Fix integer-overflow-driven heap out-of-bounds write when loading a crafted model:
  `mwt::save_load` passed a file-controlled `policies_size` to `v_array::resize`, where
  `sizeof(T) * length` could wrap and produce an allocation smaller than the recorded
  element count. The count is now bounded by the number of policy slots, and
  `v_array::reserve_nocheck` rejects any length whose byte size would overflow
  (GHSA-q2hj-ggqm-4g62)
- Fix crash when loading a crafted model with no per-model gradient-descent state:
  `save_load_online_state_gd` indexed `pms[0]` unconditionally. The vector's element
  count is deserialized from the model via ftrl, so a crafted file could empty it. The
  minimum is now established where the vector is built and restored after the
  file-controlled read (GHSA-9463-43mc-xhc6)
- Fix heap out-of-bounds read when loading a crafted model: the persisted-options field
  was appended as a C string without requiring a terminating NUL, so the scan could run
  past its allocation into adjacent heap memory. The append is now bounded by the number
  of bytes actually read (GHSA-9cm9-qvp3-c2v4)
- Complete the slates DSJSON fix from 9.11.3: an outcome supplying neither actions nor
  probabilities satisfied the length-equality check with both sizes zero and still
  reached `probabilities[0]` on an empty array. Such a slot is now treated as unlabeled
  (GHSA-c8v3-p4fg-v3pm)

### Fixed

- Bound the `_outcomes` array against the number of slot examples in the CCB DSJSON parser.
  More `_outcomes` entries than slots walked the example index off the end of the examples
  vector and set a label through the resulting pointer (#4946)
- C# bindings: retain a `SimpleLabel`'s importance weight when building examples. The weight
  was written to `example::weight`, which `VW::setup_example` reassigns from the simple-label
  reduction features, so it was reset to 1 by `CreateExample()`. Beyond reading back
  incorrectly, the supplied weight never reached the learner -- affected examples trained as
  though weighted 1.0 (#4949, reported in #4945)

## [9.11.3](https://github.com/VowpalWabbit/vowpal_wabbit/compare/9.11.2...9.11.3)

Security patch release addressing two heap out-of-bounds writes.

### Security

- Fix heap out-of-bounds write when loading a crafted model: bound the file-controlled
  legacy interaction length in `save_load_header` and bounds-check policy ids in
  `mwt::save_load`, rejecting malformed values with a clean exception before the copy
  (GHSA-x3cx-p52g-p5q7)
- Fix heap out-of-bounds write in the slates DSJSON parser: validate that the `_outcomes`,
  `_a`, and `_p` arrays are consistent before writing per-slot probabilities, instead of
  relying on `NDEBUG`-elided asserts (GHSA-c8v3-p4fg-v3pm)

## [9.11.0](https://github.com/VowpalWabbit/vowpal_wabbit/compare/9.10.0...9.11.0)

A high quality long term maintenance release. Testing across different
languages, platforms, compilers, and through test cases has increased by an
order of magnitude, cleaning up many bugs and issues found on the way. A few
minor features were added. The set of CI-supported choices are:

- **Languages**: C++, Python, Java, C#/.NET, WebAssembly/JS
- **Platforms**: Linux x64/ARM, macOS Intel/ARM, Windows x64

The WebAssembly one is particularly fun: it can run in your browser.

### Features

- Add `--cs_absolute_loss` option for CSOAA and CSOAA_LDF to report absolute cost as loss
  instead of relative (cost - min_cost) (#4883)
- Migrate Python bindings from Boost.Python to pybind11 (#4766)
- Add Python 3.10–3.13 support via cibuildwheel for Linux, macOS, and Windows (#4761, #4763)
- Add Java JNI builds for Windows (#4869)
- Add Java multi-platform publish workflow (#4866)
- Add namespace-level metrics output per reduction (#4811)
- Add TypeScript declarations for Wasm npm package (#4784)
- Expose FTRL/Coin/PiSTOL config via `get_config()` (#4780)
- Add Java support for `ActionPDFValue`, `PDF`, `ActiveMulticlass`, and `NoPred` prediction types
- Add Java `runDriver()` and `isMultiline()` JNI methods for multi-pass and multiline learning
- Add C# `ActionPdfValue` prediction type and `ContinuousActionLabel` label type for CATS
- Add C# `WorkspaceRunDriver()` and `WorkspaceRunDriverOneThread()` native APIs
- Downgrade C# bindings from .NET Standard 2.1 to 2.0 for broader compatibility (#4865)
- Enable AVX2 for Windows x64 Release builds (#4844)
- Upgrade Eigen 3.4.0 → 5.0.1; bump C++ minimum from C++11 to C++14 (#4728, #4888)

### Fixes

- Fix CSOAA_LDF holdout loss routing: use per-example `test_only` instead of global flag (#4883)
- Fix automl `insert_config` use-after-move when compacting config array (#4883)
- Fix missing `gzopen()` error handling in `gzip_file_adapter` (#4885)
- Delete unused `VW::details::print_update()` dead code (#4885)
- Add LRQ namespace collision warning at setup time (#4883)
- Convert search `fprintf(stderr)` calls to VW logger infrastructure (#4883)
- Fix search `entity_relation` crashes for `search_order` 2 and 3 (#4845)
- Fix kernel_svm assertion failure in `collision_cleanup` (#4846)
- Fix bounds checking in ECT and CSOAA_LDF reductions (#4859)
- Fix flatbuffer label parser validation (#4860)
- Fix `options_cli::replace()` parsed value update (#4857)
- Fix cover double-predict, LAS validation, shared model race, search LDF oracle (#4838)
- Fix SVRG quiet-mode bug (#4842)
- Fix pandas extension dtype handling in `dftovw` (#4789)
- Fix `.NET` target framework upgrade from netcoreapp3.1 to net8.0 (#4750)
- Fix `.NET` benchmark upgrade from net6.0 to net8.0 (#4829)
- Fix Newtonsoft.Json security vulnerability (upgrade to 13.0.3) (#4730)
- Fix deprecated `codecvt` usage with manual UTF-16/UTF-8 conversion (#4744)
- Fix Eigen JacobiSVD deprecated API usage (#4731)
- Fix FlatBuffer converter to use `unique_ptr`-based VW initialize API (#4733)
- Eliminate ~62,000 MSVC compiler warnings (#4752, #4753, #4757, #4758, #4759)
- Fix unsigned underflow in `ends_with()` (#4749)
- Fix C# pooled native example memory leak on dispose (#4891)
- Add missing `osx-arm64` runtime dependency to combined .NET NuGet package (#4894)

### CI / Build

- Replace EOL Ubuntu 18.04 CI containers with `ubuntu-latest` (gcc 7 → 13)
- Update fmt 9.1.0 → 11.0.2 and spdlog 1.11.0 → 1.15.0
- Consolidate Python wheel CI workflows into single `python_checks.yml` (#4872)
- Parallelize valgrind unit tests with integration test segments (#4873)
- Add `big_tests` regression suite to CI (#4871)
- Add Java API docs to documentation workflow (#4870)
- Add ccache to cibuildwheel macOS builds (#4858)
- Add submodule init retry to all checkout steps (#4856)
- Upgrade `actions/checkout` from v1/v3 to v6 across all workflows
- Upgrade `docker/setup-qemu-action` from v1 to v3 (#4751)
- Add flatbuffers test to vcpkg builds (#4892)
- Add static JNI build+test job to java-publish workflow (#4892)
- Extend artifact retention for native JNI and NuGet packages (#4892)

### Dependencies

Core (vendored as git submodules; override with `*_SYS_DEP=ON` CMake flags):
- Eigen 5.0.1, fmt 11.0.2, spdlog 1.15.0, RapidJSON 1.1.0, zlib 1.4.1
- Boost.Math 1.90.0 (LDA only), Armadillo 14.4.4 / Ensmallen 2.19.1 (CB graph feedback only)

Build tooling:
- CMake ≥ 3.10, C++14 (C++17 for Java/Python bindings)
- cibuildwheel 3.3.0, Python 3.10–3.14

Java (via Maven):
- JUnit 5.11.4, Gson 2.11.0, Guava 33.4.0-jre, maven-compiler-plugin 3.13.0

.NET:
- .NET Standard 2.0, Newtonsoft.Json 13.0.3

### Documentation

- Add Python configuration tutorial (#4862)
- Add examples index in `demo/README.md` and link from top-level README (#4848)
- Improve option help text for several reductions including `--cs_absolute_loss` (#4847)
- Improve `search_rollin` and `search_rollout` help text (#4805)
- Update README badges to use GitHub Actions (#4806)

### Maintenance

- Audit and resolve ~74 stale TODO/FIXME/XXX markers across the codebase (#4883, #4884, #4885, #4886)
- Convert remaining non-actionable TODOs to descriptive notes documenting design rationale

### Tests

- Add ~900 new tests across C++, E2E, Java, and Python; reported line coverage
  rose from ~82% to 90%+ but then reset to ~82% when the coverage build moved
  from gcc 7 to gcc 13 (gcc 13 instruments branch coverage more thoroughly,
  counting untaken assert/if branches as partial)
- Add C++ unit tests across loss functions, initialization, C wrapper, search,
  parsers, BFGS, LDA, kernel_svm, and many more reductions
- Add Java integration test runner (`RunTestsIT`) executing E2E tests through JNI bindings
- Unskip dozens of C# tests by adding `ActionPdfValue` and `ContinuousActionLabel` support
- Unskip Python and macOS ARM precision tests by reducing dataset sizes (#4850)
- Remove `--quiet` from tests 334–391 to improve coverage (#4851)
- Remove 26 redundant tests with identical coverage (#4830)
