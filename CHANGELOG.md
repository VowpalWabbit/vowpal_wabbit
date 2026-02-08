# Changelog

All notable changes to Vowpal Wabbit are documented in this file. For changes
prior to this file's creation, see [GitHub Releases](https://github.com/VowpalWabbit/vowpal_wabbit/releases).

## [Unreleased](https://github.com/VowpalWabbit/vowpal_wabbit/compare/9.10.0...HEAD)

### Features

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
- Support building with Eigen 5.0.0 (#4728)

### Fixes

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

### Documentation

- Add Python configuration tutorial (#4862)
- Add examples index in `demo/README.md` and link from top-level README (#4848)
- Improve option help text for several reductions (#4847)
- Improve `search_rollin` and `search_rollout` help text (#4805)
- Update README badges to use GitHub Actions (#4806)

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
