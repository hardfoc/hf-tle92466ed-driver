# =============================================================================
# hf-tle92466ed-driver: install check consumer
# =============================================================================
# A project outside the source tree that uses only the installed package:
#   find_package(hf_tle92466ed) + hf::tle92466ed. Built by tests/check_install.cmake.
# =============================================================================
cmake_minimum_required(VERSION 3.16)
project(hf_tle92466ed_install_consumer LANGUAGES CXX)

find_package(hf_tle92466ed 1.0 REQUIRED CONFIG)

add_library(consumer OBJECT main.cpp)
target_compile_features(consumer PRIVATE cxx_std_20)
target_link_libraries(consumer PRIVATE hf::tle92466ed)
