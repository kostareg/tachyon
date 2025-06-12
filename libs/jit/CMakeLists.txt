# libs/jit/CMakeLists.txt

# Platform-specific sources
set(JIT_SOURCES
        src/jit_compiler.cpp
        src/jit_memory.cpp
        src/optimization.cpp
)

# Add platform-specific code generation
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    list(APPEND JIT_SOURCES src/x86_64_codegen.cpp)
    set(JIT_ARCH_DEFINES JIT_ARCH_X86_64)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    list(APPEND JIT_SOURCES src/aarch64_codegen.cpp)
    set(JIT_ARCH_DEFINES JIT_ARCH_AARCH64)
else ()
    message(WARNING "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    list(APPEND JIT_SOURCES src/interpreter_fallback.cpp)
    set(JIT_ARCH_DEFINES JIT_ARCH_INTERPRETER)
endif ()

add_library(tachyon_jit STATIC ${JIT_SOURCES})

target_include_directories(tachyon_jit
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Dependencies - JIT needs codegen and runtime
target_link_libraries(tachyon_jit
        PUBLIC
        tachyon::codegen    # Needs IR types in public API
        PRIVATE
        tachyon::runtime    # Runtime integration
        ${CMAKE_DL_LIBS}    # Dynamic loading for JIT
)

# Platform-specific linking
if (UNIX AND NOT APPLE)
    target_link_libraries(tachyon_jit PRIVATE rt)  # Real-time extensions
endif ()

if (WIN32)
    target_link_libraries(tachyon_jit PRIVATE kernel32)
endif ()

# Compiler definitions
target_compile_definitions(tachyon_jit
        PRIVATE
        ${JIT_ARCH_DEFINES}
        TACHYON_JIT_VERSION="${PROJECT_VERSION}"
        PUBLIC
        $<$<CONFIG:Debug>:TACHYON_JIT_DEBUG>
)

# Compiler-specific flags for JIT performance
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(tachyon_jit PRIVATE
            -fno-rtti           # JIT doesn't need RTTI
            -ffast-math         # Aggressive math optimizations
            $<$<CONFIG:Release>:-march=native>  # Use all CPU features in release
    )
endif ()

# Memory protection - needed for executable code generation
if (UNIX)
    target_compile_definitions(tachyon_jit PRIVATE _GNU_SOURCE)
endif ()

add_library(tachyon::jit ALIAS tachyon_jit)

# Tests
if (TACHYON_BUILD_TESTS)
    add_executable(jit_tests
            tests/jit_compiler_test.cpp
            tests/codegen_test.cpp
            tests/memory_test.cpp
            tests/optimization_test.cpp
    )

    target_link_libraries(jit_tests
            PRIVATE
            tachyon_jit
            tachyon_codegen
            gtest
            gtest_main
    )

    add_test(NAME JITTests COMMAND jit_tests)

    # Set test timeout (JIT tests can be slow)
    set_tests_properties(JITTests PROPERTIES TIMEOUT 60)
endif ()

# Benchmarks - critical for JIT performance
if (TACHYON_BUILD_BENCHMARKS)
    add_executable(jit_benchmarks
            benchmarks/compilation_speed.cpp
            benchmarks/execution_speed.cpp
            benchmarks/memory_usage.cpp
            benchmarks/warmup_time.cpp
    )

    target_link_libraries(jit_benchmarks
            PRIVATE
            tachyon_jit
            tachyon_codegen
            benchmark::benchmark
    )

    # Benchmark data files
    file(COPY benchmarks/programs/
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/benchmark_programs/)
endif ()