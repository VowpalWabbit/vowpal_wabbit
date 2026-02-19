# Changelog

All notable changes to Vowpal Wabbit are documented in this file. For changes
prior to this file's creation, see [GitHub Releases](https://github.com/VowpalWabbit/vowpal_wabbit/releases).

## [9.11.0](https://github.com/VowpalWabbit/vowpal_wabbit/compare/9.10.0...9.11.0)

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
- Support building with Eigen 5.0.0 (#4728)

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

### Dependencies

Core (vendored as git submodules; override with `*_SYS_DEP=ON` CMake flags):
- Eigen 3.4.0, fmt 11.0.2, spdlog 1.15.0, RapidJSON 1.1.0, zlib 1.4.1
- Boost.Math 1.90.0 (LDA only), Armadillo 14.4.4 / Ensmallen 2.19.1 (CB graph feedback only)

Build tooling:
- CMake ≥ 3.10, C++11 (C++17 for Java/Python bindings)
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
