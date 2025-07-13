# libs/common/CMakeLists.txt
add_library(tachyon_common INTERFACE)

# Public headers - what other libraries can use
target_include_directories(tachyon_common
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Alias for consistent naming
add_library(tachyon::common ALIAS tachyon_common)

# Tests for this library
if (TACHYON_BUILD_TESTS)
    add_executable(common_tests
            tests/error_test.cpp
            tests/matrix_test.cpp
    )

    target_link_libraries(common_tests
            PRIVATE
            tachyon_common
            gtest
            gtest_main
    )

    # Add to test suite
    add_test(NAME CommonTests COMMAND common_tests)

    # Test data files
    # file(COPY tests/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
endif ()

# Benchmarks for this library
#if (TACHYON_BUILD_BENCHMARKS)
#    add_executable(common_benchmarks
#            benchmarks/common_benchmark.cpp
#    )
#
#    target_link_libraries(common_benchmarks
#            PRIVATE
#            tachyon_common
#            benchmark::benchmark
#    )
#endif ()
