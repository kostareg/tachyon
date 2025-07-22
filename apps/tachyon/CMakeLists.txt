add_executable(tachyon_tachyon
        src/main.cpp
        src/repl.cpp
        src/run.cpp
)

target_link_libraries(tachyon_tachyon
        PRIVATE
        tachyon::common
        tachyon::lexer
        tachyon::parser
        tachyon::codegen
        tachyon::runtime
)

target_include_directories(tachyon_tachyon
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(tachyon_tachyon
        PRIVATE
        TACHYON_COMPILER_VERSION="${PROJECT_VERSION}"
        TACHYON_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)

set_target_properties(tachyon_tachyon PROPERTIES
        OUTPUT_NAME tachyon
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS tachyon_tachyon
        RUNTIME DESTINATION bin
)

if (UNIX)
    add_custom_command(TARGET tachyon_tachyon POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/bin/tachyon
            ${CMAKE_BINARY_DIR}/tachyon
            COMMENT "Creating development symlink"
    )
endif ()