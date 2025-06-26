add_executable(benchmarks
        pipeline_benchmark.cpp)

target_link_libraries(benchmarks
        PRIVATE
        tachyon::common
        tachyon::lexer
        tachyon::parser
        tachyon::codegen
        tachyon::runtime
        benchmark::benchmark
)

target_include_directories(benchmarks
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../libs/common/include
        ../libs/lexer/include
        ../libs/parser/include
        ../libs/codegen/include
        ../libs/runtime/include
)
