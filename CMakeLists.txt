cmake_minimum_required(VERSION 3.16)
project(tachyon
        VERSION 1.0.0
        DESCRIPTION "A just-in-time compiler optimized for mathematical operations."
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
endif ()

option(TACHYON_BUILD_TESTS "Build tests" ON)
option(TACHYON_BUILD_BENCHMARKS "Build benchmarks" ON)
option(TACHYON_BUILD_BENCHMARKS "Build code coverage" OFF)
option(TACHYON_BUILD_TOOLS "Build apps" ON)
option(TACHYON_ENABLE_ASAN "Enable AddressSanitizer" OFF)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(TY_DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(TY_RELEASE)
endif ()

if (TACHYON_BUILD_TESTS)
    find_package(GTest QUIET)
    if (NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
                googletest
                GIT_REPOSITORY https://github.com/google/googletest.git
                GIT_TAG v1.16.0
        )
        FetchContent_MakeAvailable(googletest)
    endif ()
    enable_testing()
endif ()

if (TACHYON_BUILD_BENCHMARKS)
    # disable internal benchmark tests
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE BOOL "" FORCE)

    include(FetchContent)
    FetchContent_Declare(
            googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.9.4
    )
    FetchContent_MakeAvailable(googlebenchmark)

    # optional: forcibly unset any benchmark test registration with CTest
    set_property(DIRECTORY "${benchmark_SOURCE_DIR}" PROPERTY TEST_INCLUDE_FILE "")
endif ()

if (TACHYON_BUILD_WITH_COVERAGE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif ()
endif ()

add_subdirectory(libs/common)
add_subdirectory(libs/lexer)
add_subdirectory(libs/parser)
add_subdirectory(libs/codegen)
add_subdirectory(libs/runtime)

if (TACHYON_BUILD_TOOLS)
    add_subdirectory(apps/tachyon)
endif ()

# for integration tests only.
if (TACHYON_BUILD_TESTS)
    add_subdirectory(tests)
endif ()

# for global benchmarks only.
if (TACHYON_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()

install(TARGETS tachyon_common tachyon_lexer tachyon_parser
        tachyon_codegen tachyon_runtime
        EXPORT tachyonTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

install(DIRECTORY libs/common/include/ DESTINATION include)
install(DIRECTORY libs/lexer/include/ DESTINATION include)
install(DIRECTORY libs/parser/include/ DESTINATION include)
install(DIRECTORY libs/codegen/include/ DESTINATION include)
install(DIRECTORY libs/runtime/include/ DESTINATION include)

install(EXPORT tachyonTargets
        FILE tachyonTargets.cmake
        NAMESPACE tachyon::
        DESTINATION lib/cmake/tachyon
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        tachyonConfigVersion.cmake
        VERSION ${PACKAGE_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(FILES tachyonConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/tachyonConfigVersion.cmake
        DESTINATION lib/cmake/tachyon
)