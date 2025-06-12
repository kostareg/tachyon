# tools/tachyon/CMakeLists.txt

add_executable(tachyon_tachyon
        src/main.cpp
        src/repl.cpp
        src/run.cpp
)

# The tachyon tool uses ALL libraries
target_link_libraries(tachyon_tachyon
        PRIVATE
        tachyon::common
        tachyon::lexer
        tachyon::parser
        tachyon::runtime
        #tachyon::parser
        #tachyon::analyzer
        #tachyon::codegen
        #tachyon::jit
        #tachyon::runtime
)

target_include_directories(tachyon_tachyon
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler-specific definitions
target_compile_definitions(tachyon_tachyon
        PRIVATE
        TACHYON_COMPILER_VERSION="${PROJECT_VERSION}"
        TACHYON_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)

# Set executable name
set_target_properties(tachyon_tachyon PROPERTIES
        OUTPUT_NAME tachyon
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install the tachyon
install(TARGETS tachyon_tachyon
        RUNTIME DESTINATION bin
)

# Create a symlink for development
if (UNIX)
    add_custom_command(TARGET tachyon_tachyon POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/bin/tachyon
            ${CMAKE_BINARY_DIR}/tachyon
            COMMENT "Creating development symlink"
    )
endif ()

# Integration tests for the full tachyon
#if(TACHYON_BUILD_TESTS)
#    add_executable(tachyon_integration_tests
#            tests/integration_test.cpp
#            tests/end_to_end_test.cpp
#            tests/error_handling_test.cpp
#    )
#
#    target_link_libraries(tachyon_integration_tests
#            PRIVATE
#            tachyon::lexer
#            tachyon::parser
#            tachyon::analyzer
#            tachyon::codegen
#            tachyon::jit
#            tachyon::runtime
#            gtest
#            gtest_main
#    )
#
#    add_test(NAME CompilerIntegrationTests
#            COMMAND tachyon_integration_tests)
#
#    # Test programs
#    file(COPY tests/programs/
#            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test_programs/)
#endif()