# libs/parser/CMakeLists.txt
add_library(tachyon_parser STATIC
        src/parser.cpp
        src/print.cpp
)

# Public headers - what other libraries can use
target_include_directories(tachyon_parser
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler-specific optimizations
target_compile_features(tachyon_parser PUBLIC cxx_std_23)

# Preprocessor definitions
target_compile_definitions(tachyon_parser
        PRIVATE
        TACHYON_PARSER_VERSION="${PROJECT_VERSION}"
        PUBLIC
        $<$<CONFIG:Debug>:TACHYON_DEBUG>
)

target_link_libraries(tachyon_parser
        PRIVATE
        tachyon::common
        tachyon::lexer
        tachyon::runtime
)

# Alias for consistent naming
add_library(tachyon::parser ALIAS tachyon_parser)

# Tests for this library
#if (TACHYON_BUILD_TESTS)
#    add_executable(parser_tests
#            tests/parser_test.cpp
#            tests/token_test.cpp
#            tests/unicode_test.cpp
#    )
#
#    target_link_libraries(parser_tests
#            PRIVATE
#            tachyon_parser
#            gtest
#            gtest_main
#    )
#
#    # Add to test suite
#    add_test(NAME LexerTests COMMAND parser_tests)
#
#    # Test data files
#    #file(COPY tests/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
#endif ()

# Benchmarks for this library
#if (TACHYON_BUILD_BENCHMARKS)
#    add_executable(parser_benchmarks
#            benchmarks/parser_benchmark.cpp
#    )
#
#    target_link_libraries(parser_benchmarks
#            PRIVATE
#            tachyon_parser
#            benchmark::benchmark
#    )
#endif ()