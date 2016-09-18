# CMakeLists.txt
#
# Copyright (c) 2016 Frank Lin (lin.xiaoe.f@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Build lerc_utils on macOS or linux.

cmake_minimum_required (VERSION 2.6)

project (lerctiler)

file (GLOB core_src ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cc)

# Build liblerc.
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/vendor/lerc/src)
add_library (tiff STATIC IMPORTED)

add_executable (lerctiler ${core_src} ${CMAKE_CURRENT_SOURCE_DIR}/proj.cmd/main.cc)

if (APPLE) # macOS (iOS is not supported)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/core
                       ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lerc/include/mac
                       ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libtiff/include/mac)

  set (BIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/mac)
  set (CMAKE_CXX_FLAGS "-std=c++11")

  add_definitions(-DSKR_PLATFORM=SKR_PLATFORM_MAC)

  target_link_libraries (lerctiler ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libtiff/prebuilt/mac/libtiff.a)
  target_link_libraries (lerctiler ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lerc/prebuilt/mac/liblerc.a)
elseif (UNIX AND NOT APPLE) # linux
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/core
                       ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lerc/include/linux
                       ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libtiff/include/linux)

  set (BIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux)
  set (CMAKE_CXX_FLAGS "-std=c++11")

  add_definitions(-DSKR_PLATFORM=SKR_PLATFORM_LINUX)

  target_link_libraries (lerctiler ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libtiff/prebuilt/linux/64-bit/libtiff.a)
  target_link_libraries (lerctiler ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lerc/prebuilt/linux/liblerc.a)
endif (APPLE)

# link native libz
find_package (ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(lerctiler ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

install (TARGETS lerctiler DESTINATION ${BIN_PATH})
