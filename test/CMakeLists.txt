include(gtest)

add_executable(test_spinlock test_spinlock.cpp)
add_executable(test_seqlock test_seqlock.cpp)

add_dependencies(test_spinlock gtest)
target_link_libraries(test_spinlock libgtest.a libgtest_main.a)

enable_testing()
add_test(NAME test_spinlock COMMAND test_spinlock)
add_test(NAME test_seqlock COMMAND test_seqlock)

add_subdirectory(bench)