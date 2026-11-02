#===============================================================================
# TLE92466ED Driver - Root CMake
# Defines the main build target for the driver. Minimal, platform-agnostic.
#===============================================================================

cmake_minimum_required(VERSION 3.16)

project(hf_tle92466ed_driver LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/hf_tle92466ed_build_settings.cmake)
include(GNUInstallDirs)

# Installed layout keeps the source tree's inc/ + src/ pair, because tle92466ed.hpp includes
# "../src/tle92466ed.ipp" relative to itself:
#   <prefix>/include/hf_tle92466ed/inc/*.hpp, tle92466ed_version.h
#   <prefix>/include/hf_tle92466ed/src/tle92466ed.ipp
set(HF_TLE92466ED_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/hf_tle92466ed/inc")

if(HF_TLE92466ED_SOURCE_FILES)
    add_library(${HF_TLE92466ED_TARGET_NAME} STATIC ${HF_TLE92466ED_SOURCE_FILES})
    target_include_directories(${HF_TLE92466ED_TARGET_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
            $<BUILD_INTERFACE:${HF_TLE92466ED_VERSION_HEADER_DIR}>
            $<INSTALL_INTERFACE:${HF_TLE92466ED_INSTALL_INCLUDEDIR}>
    )
    target_compile_features(${HF_TLE92466ED_TARGET_NAME} PUBLIC cxx_std_20)
else()
    add_library(${HF_TLE92466ED_TARGET_NAME} INTERFACE)
    target_include_directories(${HF_TLE92466ED_TARGET_NAME}
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
            $<BUILD_INTERFACE:${HF_TLE92466ED_VERSION_HEADER_DIR}>
            $<INSTALL_INTERFACE:${HF_TLE92466ED_INSTALL_INCLUDEDIR}>
    )
    target_compile_features(${HF_TLE92466ED_TARGET_NAME} INTERFACE cxx_std_20)
endif()

add_library(hf::tle92466ed ALIAS ${HF_TLE92466ED_TARGET_NAME})

# EXPORT_NAME makes the installed target hf::tle92466ed, the same name as the build-tree alias.
set_target_properties(${HF_TLE92466ED_TARGET_NAME} PROPERTIES
    VERSION ${HF_TLE92466ED_VERSION}
    SOVERSION ${HF_TLE92466ED_VERSION_MAJOR}
    EXPORT_NAME tle92466ed)

option(HF_TLE92466ED_ENABLE_WARNINGS "Enable extra warnings for TLE92466ED driver" OFF)
if(HF_TLE92466ED_ENABLE_WARNINGS)
    if(TARGET ${HF_TLE92466ED_TARGET_NAME})
        target_compile_options(${HF_TLE92466ED_TARGET_NAME} INTERFACE -Wall -Wextra -Wpedantic)
    endif()
endif()

#===============================================================================
# Host unit tests (register model of the chip; no hardware)
#===============================================================================
option(HF_TLE92466ED_BUILD_TESTS "Build the host unit tests in tests/" OFF)
if(HF_TLE92466ED_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include(CMakePackageConfigHelpers)

install(TARGETS ${HF_TLE92466ED_TARGET_NAME} EXPORT hf_tle92466edTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${HF_TLE92466ED_INSTALL_INCLUDEDIR}
)

# Hand-written headers, the generated version header and the template implementation the
# header includes (src/tle92466ed.ipp), in the layout described at the top of this file.
install(DIRECTORY inc/ DESTINATION ${HF_TLE92466ED_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(FILES "${HF_TLE92466ED_VERSION_HEADER}" DESTINATION ${HF_TLE92466ED_INSTALL_INCLUDEDIR})
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hf_tle92466ed/src
    FILES_MATCHING PATTERN "*.ipp")

install(EXPORT hf_tle92466edTargets
    FILE hf_tle92466edTargets.cmake
    NAMESPACE hf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_tle92466ed
)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/hf_tle92466edConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/hf_tle92466edConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_tle92466ed
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/hf_tle92466edConfigVersion.cmake"
    VERSION ${HF_TLE92466ED_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/hf_tle92466edConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/hf_tle92466edConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_tle92466ed
)
