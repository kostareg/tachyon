add_executable(integration_tests
        integration/array_test.cpp
        integration/comparison_test.cpp
        integration/pipeline_test.cpp
)

target_link_libraries(integration_tests
        PRIVATE
        tachyon::common
        tachyon::lexer
        tachyon::parser
        tachyon::codegen
        tachyon::runtime
        gtest
        gtest_main
)

target_include_directories(integration_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../libs/common/include
        ../libs/lexer/include
        ../libs/parser/include
        ../libs/codegen/include
        ../libs/runtime/include
)

add_test(NAME IntegrationTests COMMAND integration_tests)

set_tests_properties(IntegrationTests PROPERTIES TIMEOUT 120)

if (TACHYON_BUILD_TESTS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    option(TACHYON_ENABLE_COVERAGE "Enable coverage reporting" OFF)

    if (TACHYON_ENABLE_COVERAGE)
        target_compile_options(integration_tests PRIVATE --coverage)
        target_link_options(integration_tests PRIVATE --coverage)

        find_program(GCOV gcov)
        find_program(LCOV lcov)
        find_program(GENHTML genhtml)

        if (GCOV AND LCOV AND GENHTML)
            add_custom_target(coverage
                    COMMAND ${CMAKE_MAKE_PROGRAM} test
                    COMMAND ${LCOV} --capture --directory . --output-file coverage.info
                    COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/tests/*' '*/build/*' --output-file coverage.info
                    COMMAND ${GENHTML} coverage.info --output-directory coverage_report
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Generating coverage report"
            )
        endif ()
    endif ()
endif ()