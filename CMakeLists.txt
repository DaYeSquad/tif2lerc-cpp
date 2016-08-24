cmake_minimun_required (VERSION 2.6)
project(LercTiler)

include_directories(LercTiler/lerc/include)

link_directories(LercTiler/vendor/libtiff/prebuilt/mac)

add_subdirectory(core)
