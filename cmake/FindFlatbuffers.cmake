# Copyright 2014 Stefan.Eilemann@epfl.ch
# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Find the flatbuffers schema compiler
#
# Output Variables:
# * FLATBUFFERS_FLATC_EXECUTABLE the flatc compiler executable
# * FLATBUFFERS_FOUND
#

# File modified from original contents to remove FLATBUFFERS_GENERATE_C_HEADERS and inclusion of BuildFlatBuffers.cmake

set(FLATBUFFERS_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

find_program(FLATBUFFERS_FLATC_EXECUTABLE NAMES flatc)
find_path(FLATBUFFERS_INCLUDE_DIR NAMES flatbuffers/flatbuffers.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Flatbuffers
  DEFAULT_MSG FLATBUFFERS_FLATC_EXECUTABLE FLATBUFFERS_INCLUDE_DIR)

if(FLATBUFFERS_FOUND)
  set(FLATBUFFERS_INCLUDE_DIRS ${FLATBUFFERS_INCLUDE_DIR})
  include_directories(${CMAKE_BINARY_DIR})
else()
  set(FLATBUFFERS_INCLUDE_DIR)
endif()
