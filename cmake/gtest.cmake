##################################
# Download and install GoogleTest

set(GTEST_GIT_URL https://github.com/google/googletest.git)
set(GTEST_GIT_TAG release-1.10.0)

include(ExternalProject)
ExternalProject_Add(gtest
    GIT_REPOSITORY "${GTEST_GIT_URL}"
    GIT_TAG "${GTEST_GIT_TAG}"
    PREFIX "${CMAKE_BINARY_DIR}/3rdparty/gtest"
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)
ExternalProject_Get_Property(gtest SOURCE_DIR BINARY_DIR)

set(GTEST_SOURCE_DIR ${SOURCE_DIR})
set(GTEST_BINARY_DIR ${BINARY_DIR})

include_directories(${GTEST_SOURCE_DIR}/googletest/include)
include_directories(${GTEST_SOURCE_DIR}/googlemock/include)
