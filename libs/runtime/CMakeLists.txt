add_library(tachyon_runtime STATIC
        src/value.cpp
        src/cache.cpp
        src/vm.cpp
)

target_include_directories(tachyon_runtime
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tachyon_runtime PUBLIC cxx_std_23)

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
        tachyon::codegen
)

add_library(tachyon::runtime ALIAS tachyon_runtime)

if (TACHYON_BUILD_BENCHMARKS)
    add_executable(runtime_benchmarks
            benchmarks/vm_benchmark.cpp
    )

    target_link_libraries(runtime_benchmarks
            PRIVATE
            tachyon_common
            tachyon_runtime
            benchmark::benchmark
    )

    file(COPY benchmarks/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/benchmark-data/)

    set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/data
    )
endif ()