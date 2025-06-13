# libs/codegen/CMakeLists.txt
add_library(tachyon_codegen STATIC
        src/bytecode_generator.cpp
)

# Public headers - what other libraries can use
target_include_directories(tachyon_codegen
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler-specific optimizations
target_compile_features(tachyon_codegen PUBLIC cxx_std_23)

# Preprocessor definitions
target_compile_definitions(tachyon_codegen
        PRIVATE
        TACHYON_CODEGEN_VERSION="${PROJECT_VERSION}"
        PUBLIC
        $<$<CONFIG:Debug>:TACHYON_DEBUG>
)

target_link_libraries(tachyon_codegen
        PRIVATE
        tachyon::common
        tachyon::parser
        tachyon::runtime
)

# Alias for consistent naming
add_library(tachyon::codegen ALIAS tachyon_codegen)

# Tests for this library
#if (TACHYON_BUILD_TESTS)
#    add_executable(codegen_tests
#            tests/codegen_test.cpp
#            tests/token_test.cpp
#            tests/unicode_test.cpp
#    )
#
#    target_link_libraries(codegen_tests
#            PRIVATE
#            tachyon_codegen
#            gtest
#            gtest_main
#    )
#
#    # Add to test suite
#    add_test(NAME LexerTests COMMAND codegen_tests)
#
#    # Test data files
#    #file(COPY tests/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
#endif ()

# Benchmarks for this library
#if (TACHYON_BUILD_BENCHMARKS)
#    add_executable(codegen_benchmarks
#            benchmarks/codegen_benchmark.cpp
#    )
#
#    target_link_libraries(codegen_benchmarks
#            PRIVATE
#            tachyon_codegen
#            benchmark::benchmark
#    )
#endif ()