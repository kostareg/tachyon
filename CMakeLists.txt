cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
project(tachyon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable modules
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

# Declare modules
add_executable(tachyon
  src/main.cpp
)

target_sources(tachyon
  PRIVATE
    src/ast/ast.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/vm/vm.cpp
)

target_sources(tachyon
  PRIVATE
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
      src/op.cppm
      src/error.cppm
      src/lexer/lexer.cppm
      src/parser/parser.cppm
      src/ast/ast.cppm
      src/vm/vm.cppm
)
