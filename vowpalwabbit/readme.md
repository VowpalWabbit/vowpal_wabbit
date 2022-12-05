# VowpalWabbit Components

## Renames
Some common targets were renamed to match the new consistent style:

- `vw-bin` -> `vw_cli_bin`
- `vw` -> `vw_core`
- `spanning_tree` -> `vw_spanning_tree_bin`

## Component descriptions

| name              | target                   | type        | description                                                                                                      | public_deps                                                              | private_deps                                    | exceptions                                 |
| ----------------- | ------------------------ | ----------- | ---------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------ | ----------------------------------------------- | ------------------------------------------ |
| active_interactor | vw_active_interactor_bin | EXECUTABLE  | Tool for interacting with active mode reductions                                                                 |                                                                          |                                                 | N/A                                        |
| allreduce         | vw_allreduce             | STATIC_ONLY | Supporting library for thread or socket based distributed learning                                               | vw_common, vw_io                                                         |                                                 | Yes                                        |
| c_wrapper         | vw_c_wrapper             | SHARED_ONLY | Old C API. Cannot convey errors in current interface. Can be disabled using option `VW_BUILD_VW_C_WRAPPER`       |                                                                          | vw_core                                         | Yes, exceptions are thrown across boundary |
| cli               | vw_cli_bin               | EXECUTABLE  | Primary VW command line interface. The `vw` executable.                                                          |                                                                          | vw_core                                         | N/A                                        |
| common            | vw_common                | HEADER_ONLY | Common utilities that are shared by every project. The only dependencies permitted are polyfill/vocabulary types | nonstd::string-view-lite                                                         |                                                 | Yes, also supports `VW_NOEXCEPT`           |
| config            | vw_config                | STATIC_ONLY | Option parsing, and command line utilities                                                                       | vw_common                                                                | fmt::fmt                                        | Yes                                        |
| core              | vw_core                  | STATIC_ONLY | This contains all remaining VW code, all reduction implementations, driver, option handling                      | vw_common, vw_explore, vw_allreduce, vw_config, spdlog::spdlog, fmt::fmt | dl, Threads::Threads, vw_io, Boost::math, eigen, RapidJSON | Yes                                         |
| csv_parser | vw_csv_parser | STATIC_ONLY | Parser implementation that reads csv examples. Disabled by default. Enable with `VW_BUILD_CSV` | vw_common, vw_config, vw_core |              | Yes        |
| explore           | vw_explore               | HEADER_ONLY | Utilities for sampling and generating exploration distributions                                                  | vw_common                                                                |                                                 | No                                         |
| io                | vw_io                    | STATIC_ONLY | Utilities for input and output                                                                                   | vw_common, spdlog::spdlog, fmt::fmt                                      | ZLIB::ZLIB                                      | Yes                                        |
| slim              | vw_slim                  | STATIC_ONLY | Minimal inference only runtime                                                                                   | vw_common, vw_explore                                                    |                                                 | No                                         |
| spanning_tree     | vw_spanning_tree_bin     | EXECUTABLE  | Command line tool for connecting instances of vw for distributed learning                                        |                                                                          | vw_spanning_tree, vw_common, vw_config          | N/A                                        |
| spanning_tree     | vw_spanning_tree         | STATIC_ONLY | Supporting code for connecting instances of VW for distributed learning                                          | vw_common                                                                | Threads::Threads                                | Yes                                        |
| fb_parser         | vw_fb_parser             | STATIC_ONLY | Parser implementation that reads flatbuffer examples. Disabled by default. Enable with `BUILD_FLATBUFFERS`.                                                             | vw_core, fb_generate_headers                                             |                                                 | Yes                                        |

How to generate the above table:
1. Run CMake configure with `-DVW_OUPUT_LIB_DESCRIPTIONS=On`
2. Pass the generated JSON through [this website](https://kdelmonte.github.io/json-to-markdown-table/)
