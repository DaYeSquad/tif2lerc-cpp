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

# Build liblerc on macOS or linux.

cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 11)

file (GLOB lerc_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file (GLOB lerc1_decode_src ${CMAKE_CURRENT_SOURCE_DIR}/src/Lerc1Decode/*.cpp)

file (GLOB common_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/Common/*.h)
file (GLOB lerc_headrs ${CMAKE_CURRENT_SOURCE_DIR}/src/Lerc/*.h)
file (GLOB lerc1_deocde_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/Lerc1Decode/*.h)
file (GLOB lerc2_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/Lerc2/*.h)

add_library (lerc ${common_src} ${lerc_src} ${lerc1_decode_src} ${lerc2_src})

if (APPLE)
  set(PREBUILT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../prebuilt/mac)
  set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../include/mac)
elseif (UNIX AND NOT APPLE)
  set(PREBUILT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../prebuilt/linux)
  set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../include/linux)
endif (APPLE)

install (TARGETS lerc DESTINATION ${PREBUILT_PATH})
install (FILES ${common_headers} DESTINATION ${INCLUDE_PATH}/Common)
install (FILES ${lerc_headrs} DESTINATION ${INCLUDE_PATH}/Lerc)
install (FILES ${lerc1_deocde_headers} DESTINATION ${INCLUDE_PATH}/Lerc1Decode)
install (FILES ${lerc2_headers} DESTINATION ${INCLUDE_PATH}/Lerc2)
