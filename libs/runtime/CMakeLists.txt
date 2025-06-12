# libs/runtime/CMakeLists.txt
add_library(tachyon_runtime STATIC
        src/vm.cpp
)

# Public headers - what other libraries can use
target_include_directories(tachyon_runtime
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler-specific optimizations
target_compile_features(tachyon_runtime PUBLIC cxx_std_23)

# Preprocessor definitions
target_compile_definitions(tachyon_runtime
        PRIVATE
        TACHYON_RUNTIME_VERSION="${PROJECT_VERSION}"
        PUBLIC
        $<$<CONFIG:Debug>:TACHYON_DEBUG>
)

target_link_libraries(tachyon_runtime
        PRIVATE
        tachyon::common
        tachyon::parser
)

# Alias for consistent naming
add_library(tachyon::runtime ALIAS tachyon_runtime)

# Tests for this library
#if (TACHYON_BUILD_TESTS)
#    add_executable(runtime_tests
#            tests/runtime_test.cpp
#            tests/token_test.cpp
#            tests/unicode_test.cpp
#    )
#
#    target_link_libraries(runtime_tests
#            PRIVATE
#            tachyon_runtime
#            gtest
#            gtest_main
#    )
#
#    # Add to test suite
#    add_test(NAME LexerTests COMMAND runtime_tests)
#
#    # Test data files
#    #file(COPY tests/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
#endif ()

# Benchmarks for this library
#if (TACHYON_BUILD_BENCHMARKS)
#    add_executable(runtime_benchmarks
#            benchmarks/runtime_benchmark.cpp
#    )
#
#    target_link_libraries(runtime_benchmarks
#            PRIVATE
#            tachyon_runtime
#            benchmark::benchmark
#    )
#endif ()