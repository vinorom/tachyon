include(benchmark)

add_executable(bench_spinlock bench_spinlock.cpp)

add_dependencies(bench_spinlock benchmark)
target_link_libraries(bench_spinlock libbenchmark.a libbenchmark_main.a)
