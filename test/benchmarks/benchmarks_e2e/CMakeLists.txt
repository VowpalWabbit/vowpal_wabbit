add_executable(vw-benchmarks-e2e.out
  e2e_benchmark_main.cc
  e2e_rcv1_benchmarks.cc
  benchmark_text_input.cc
)

find_package(benchmark REQUIRED)

# Add the include directories from vw target for testing
target_include_directories(vw-benchmarks-e2e.out PRIVATE $<TARGET_PROPERTY:vw,INCLUDE_DIRECTORIES>)
target_link_libraries(vw-benchmarks-e2e.out PRIVATE vw benchmark::benchmark)

# Communicate that Boost Unit Test is being statically linked
if(STATIC_LINK_VW)
  target_compile_definitions(vw-benchmarks-e2e.out PRIVATE STATIC_LINK_VW)
endif()

add_test(
  NAME vw_benchmarks_e2e
  COMMAND ./vw-benchmarks-e2e.out
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)