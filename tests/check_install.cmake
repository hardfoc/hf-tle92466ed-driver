# =============================================================================
# hf-tle92466ed-driver: install check (run by CTest as hf_tle92466ed_install)
# =============================================================================
# Installs the driver from BUILD_DIR into a scratch prefix, then configures and builds
# tests/install_consumer against that prefix only. Fails if the package is incomplete (a missing
# header, version header or src/tle92466ed.ipp) or exports the wrong target name.
#
# Variables: BUILD_DIR, SOURCE_DIR, WORK_DIR, CXX_COMPILER, GENERATOR
# =============================================================================
file(REMOVE_RECURSE "${WORK_DIR}")
set(prefix "${WORK_DIR}/prefix")

function(run)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE rc)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Install check failed (${rc}): ${ARGN}")
  endif()
endfunction()

run("${CMAKE_COMMAND}" --install "${BUILD_DIR}" --prefix "${prefix}")
run("${CMAKE_COMMAND}" -S "${SOURCE_DIR}/tests/install_consumer" -B "${WORK_DIR}/consumer"
    -G "${GENERATOR}" "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}" "-DCMAKE_PREFIX_PATH=${prefix}"
    -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF)
run("${CMAKE_COMMAND}" --build "${WORK_DIR}/consumer")
message(STATUS "Install check passed: ${prefix}")
