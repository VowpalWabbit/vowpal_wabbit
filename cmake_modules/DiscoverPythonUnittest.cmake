# Extract the test by running python code 

macro (discover_python_unittest PATH)
	execute_process(COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_VW_MODULE_PATH}/discover_python_unittest.py" ${PATH}
					  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					  OUTPUT_VARIABLE STR_TESTS
					  OUTPUT_STRIP_TRAILING_WHITESPACE
					  ERROR_STRIP_TRAILING_WHITESPACE)

	separate_arguments(TEST_LIST UNIX_COMMAND ${STR_TESTS})

	foreach(ATEST ${TEST_LIST})
		add_test(${ATEST} ${PYTHON_EXECUTABLE} "-m" "unittest" "-v" ${ATEST})
	endforeach(ATEST)
endmacro()
