##################################
# Download and install google-benchmark

if(BENCHMARK_SOURCE_DIR AND BENCHMARK_BINARY_DIR)
    return()
endif()

include(gtest)

set(BENCHMARK_GIT_URL https://github.com/google/benchmark.git)
set(BENCHMARK_GIT_TAG master)

include(ExternalProject)
ExternalProject_Add(benchmark
    DEPENDS gtest
    GIT_REPOSITORY  "${BENCHMARK_GIT_URL}"
    GIT_TAG         "${BENCHMARK_GIT_TAG}"
    PREFIX          "${CMAKE_BINARY_DIR}/3rdparty"
    CMAKE_ARGS      "-DGOOGLETEST_PATH=${GTEST_SOURCE_DIR}"
    INSTALL_COMMAND ""
    LOG_DOWNLOAD    ON
    LOG_CONFIGURE   ON
    LOG_BUILD       ON
)
ExternalProject_Get_Property(benchmark SOURCE_DIR BINARY_DIR)

set(BENCHMARK_SOURCE_DIR ${SOURCE_DIR})
set(BENCHMARK_BINARY_DIR ${BINARY_DIR})

message(STATUS "Using google-benchmark: BENCHMARK_SOURCE_DIR=${BENCHMARK_SOURCE_DIR}")
message(STATUS "Using google-benchmark: BENCHMARK_BINARY_DIR=${BENCHMARK_BINARY_DIR}")

include_directories(${BENCHMARK_SOURCE_DIR}/include)

link_directories("${BENCHMARK_BINARY_DIR}/src")
