add_library(tachyon_codegen STATIC
        src/bytecode_generator.cpp
        src/ir_generator.cpp
        src/machine_generator.cpp
)

target_include_directories(tachyon_codegen
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tachyon_codegen PUBLIC cxx_std_23)

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

add_library(tachyon::codegen ALIAS tachyon_codegen)

if (TACHYON_BUILD_BENCHMARKS)
    add_executable(codegen_benchmarks
            benchmarks/codegen_benchmark.cpp
    )

    target_link_libraries(codegen_benchmarks
            PRIVATE
            tachyon_common
            tachyon_parser
            tachyon_runtime
            tachyon_codegen
            benchmark::benchmark
    )

    file(COPY benchmarks/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/benchmark-data/)

    set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/data
    )
endif ()
