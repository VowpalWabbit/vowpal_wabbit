set(all_sources
  benchmark_main.cc
  standalone/benchmark_text_input.cc
  standalone/rcv1_benchmarks.cc
)

if (NOT BUILD_ONLY_STANDALONE_BENCHMARKS)
  set(all_sources ${all_sources}
    input_format_benchmarks.cc
    benchmark_funcs.cc
    benchmark_epsilon_decay.cc
    ../../vowpalwabbit/core/tests/simulator.cc

    # These are just for benchmarking specific standard library operations
    #benchmark_std_function.cc
    #benchmark_pointers.cc
  )
endif()

find_package(benchmark REQUIRED)

vw_add_executable(
    NAME "benchmarks"
    OVERRIDE_BIN_NAME "vw-benchmarks.out"
    SOURCES ${all_sources}
    DEPS vw_core benchmark::benchmark
    DESCRIPTION "Benchmarks"
)

# Communicate that Boost Unit Test is being statically linked
if(STATIC_LINK_VW)
  target_compile_definitions(vw_benchmarks_bin PRIVATE STATIC_LINK_VW)
endif()

add_test(
  NAME vw_benchmarks
  COMMAND ./vw-benchmarks.out
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
