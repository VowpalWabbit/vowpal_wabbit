cmake_minimum_required (VERSION 3.5)
project (pylibvw)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost ${Boost_VERSION} COMPONENTS python)
find_package(PythonLibs REQUIRED)

include_directories(${vw_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS}) 

add_library(${PROJECT_NAME} SHARED pylibvw.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE vw ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})

# strip lib prefix to keep setup.py happy
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PREFIX "")

find_program(PYTHON "python")

if(PYTHON)
    configure_file(setup.py.in setup.py)

    SET(PYTHON-TARBALL "dist/vowpalwabbit-${PACKAGE_VERSION}.tar.gz")

    add_custom_command(OUTPUT ${PYTHON-TARBALL}
                       COMMAND ${PYTHON} setup.py sdist
                       DEPENDS setup.py.in)
    add_custom_target(python-package DEPENDS ${PYTHON-TARBALL})
endif()
