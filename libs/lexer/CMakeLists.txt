# libs/lexer/CMakeLists.txt
add_library(tachyon_lexer STATIC
        src/lexer.cpp
)

# depend on fast_float
include(FetchContent)
FetchContent_Declare(
        fast_float
        GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
        GIT_TAG v8.0.2
)
FetchContent_MakeAvailable(fast_float)

# Public headers - what other libraries can use
target_include_directories(tachyon_lexer
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler-specific optimizations
target_compile_features(tachyon_lexer PUBLIC cxx_std_23)

# Preprocessor definitions
target_compile_definitions(tachyon_lexer
        PRIVATE
        TACHYON_LEXER_VERSION="${PROJECT_VERSION}"
        PUBLIC
        $<$<CONFIG:Debug>:TACHYON_DEBUG>
)

target_link_libraries(tachyon_lexer
        PRIVATE
        tachyon::common
        fast_float
)

# Alias for consistent naming
add_library(tachyon::lexer ALIAS tachyon_lexer)

# Tests for this library
if (TACHYON_BUILD_TESTS)
    add_executable(lexer_tests
            #            tests/lexer_test.cpp
            tests/lexer_test.cpp
            #            tests/unicode_test.cpp
    )

    target_link_libraries(lexer_tests
            PRIVATE
            tachyon_lexer
            tachyon_common
            gtest
            gtest_main
    )

    # Add to test suite
    add_test(NAME LexerTests COMMAND lexer_tests)

    # Test data files
    #file(COPY tests/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_data/)
endif ()

# Benchmarks for this library
if (TACHYON_BUILD_BENCHMARKS)
    add_executable(lexer_benchmarks
            benchmarks/lexer_benchmark.cpp
    )

    target_link_libraries(lexer_benchmarks
            PRIVATE
            tachyon_common
            tachyon_lexer
            benchmark::benchmark
    )

    # Benchmark data files
    file(COPY benchmarks/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/benchmark-data/)

    # Watch the data files
    set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/data
    )
endif ()