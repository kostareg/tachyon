add_library(tachyon_parser STATIC
        src/parser.cpp
        src/print.cpp
)

target_include_directories(tachyon_parser
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tachyon_parser PUBLIC cxx_std_23)

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

add_library(tachyon::parser ALIAS tachyon_parser)

if (TACHYON_BUILD_BENCHMARKS)
    add_executable(parser_benchmarks
            benchmarks/parser_benchmark.cpp
    )

    target_link_libraries(parser_benchmarks
            PRIVATE
            tachyon_common
            tachyon_lexer
            tachyon_parser
            benchmark::benchmark
    )

    file(COPY benchmarks/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/benchmark-data/)

    set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/data
    )
endif ()