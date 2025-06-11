cmake_minimum_required(VERSION 3.28)
project(Tachyon)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(tachyon)
target_sources(tachyon
        PUBLIC FILE_SET CXX_MODULES FILES ${MODULE_SOURCES}
        PRIVATE ${SOURCES}
)
