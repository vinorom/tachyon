##################################
# Download and install google-test

if(GTEST_SOURCE_DIR AND GTEST_BINARY_DIR)
  return()
endif()

set(GTEST_GIT_URL   https://github.com/google/googletest.git)
set(GTEST_GIT_TAG   release-1.10.0)

include(ExternalProject)
ExternalProject_Add(gtest
  GIT_REPOSITORY    "${GTEST_GIT_URL}"
  GIT_TAG           "${GTEST_GIT_TAG}"
  PREFIX            "${CMAKE_BINARY_DIR}/3rdparty"
  INSTALL_COMMAND   ""
  LOG_DOWNLOAD      ON
  LOG_CONFIGURE     ON
  LOG_BUILD         ON
)
ExternalProject_Get_Property(gtest SOURCE_DIR BINARY_DIR)

set(GTEST_SOURCE_DIR "${SOURCE_DIR}")
set(GTEST_BINARY_DIR "${BINARY_DIR}")

message(STATUS "Using google-test: GTEST_SOURCE_DIR=${GTEST_SOURCE_DIR}")
message(STATUS "Using google-test: GTEST_BINARY_DIR=${GTEST_BINARY_DIR}")

include_directories(${GTEST_SOURCE_DIR}/googletest/include)
include_directories(${GTEST_SOURCE_DIR}/googlemock/include)

link_directories("${GTEST_BINARY_DIR}/lib")
