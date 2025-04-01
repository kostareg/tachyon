cmake_minimum_required(VERSION 3.16)
project(tachyon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find packages
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

# Source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

include_directories(src)

add_executable(tachyon ${SRC_FILES})

# Link dependencies
target_link_libraries(tachyon
  PRIVATE
    spdlog::spdlog
    fmt::fmt
    mimalloc
)
