add_library(tachyon_common INTERFACE)

target_include_directories(tachyon_common
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_library(tachyon::common ALIAS tachyon_common)

if (TACHYON_BUILD_TESTS)
    add_executable(common_tests
            tests/error_test.cpp
            tests/matrix_test.cpp
    )

    target_link_libraries(common_tests
            PRIVATE
            tachyon_common
            gtest
            gtest_main
    )

    add_test(NAME CommonTests COMMAND common_tests)
endif ()