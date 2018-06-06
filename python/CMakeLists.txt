project (pylibvw)

set(CMAKE_CXX_STANDARD 11)

find_package(PythonLibs REQUIRED)
find_package(Boost COMPONENTS python)

if (PYTHON_INCLUDE_DIRS AND Boost_PYTHON_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	include_directories(${PYTHON_INCLUDE_DIRS})
	include_directories(${vw_INCLUDE_DIRS})

	add_library(${PROJECT_NAME} SHARED pylibvw.cc)

	target_link_libraries(${PROJECT_NAME} PRIVATE vw ${Boost_LIBRARIES})
endif()
