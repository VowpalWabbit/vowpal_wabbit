# Script to test vw_test_module import after build
message("=========================================")
message("CRITICAL TEST: vw_test_module (links to vw_core)")
message("=========================================")
message("Test directory: ${TEST_DIR}")

# First, list what's actually in the directory
execute_process(
  COMMAND python3 -c "import os; print('Files in test dir:', os.listdir('${TEST_DIR}'))"
  OUTPUT_VARIABLE DIR_CONTENTS
)
message("${DIR_CONTENTS}")

# Try the import with detailed error reporting
execute_process(
  COMMAND python3 -c "import sys, os; print('Python path:', sys.path); sys.path.insert(0, '${TEST_DIR}'); print('After insert:', sys.path); print('CWD:', os.getcwd()); import vw_test_module; print('SUCCESS: vw_test_module imported!'); print(vw_test_module.test_vw())"
  RESULT_VARIABLE TEST_RESULT
  OUTPUT_VARIABLE TEST_OUTPUT
  ERROR_VARIABLE TEST_ERROR
  WORKING_DIRECTORY ${TEST_DIR}
)

message("${TEST_OUTPUT}")
if(TEST_ERROR)
  message("${TEST_ERROR}")
endif()

message("=========================================")
if(TEST_RESULT EQUAL 0)
  message("vw_test_module works - VW library linking is OK!")
else()
  message("vw_test_module FAILED - VW library linking causes the issue!")
  message("Exit code: ${TEST_RESULT}")
endif()
message("=========================================")
