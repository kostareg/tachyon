# Tachyon: a JIT maths-optimized language

Tachyon is a just-in-time compiled programming language optimized for
mathematical operations. It serves as a personal project, and includes a virtual
machine, bytecode, high-level intermediate representation, and abstract syntax
tree. It will employ various optimization techniques such
as pure [function memoization](https://en.wikipedia.org/wiki/Memoization) and
[just-in-time compilation](https://en.wikipedia.org/wiki/Just-in-time_compilation)
to create fast and efficient programs during runtime.

The next steps for this project are, in no particular order:

* Support more just-in-time compilation constructs
    * More mathematical operations
    * Leaving/rejoining the call stack
* Complete grammar
    * High priority:
        * While loop and conditionals:
            * Continue/break
    * Medium priority:
        * Enums
        * Structs
        * Imports
        * return semantics (ending expr -> RETC/RETR)
    * Low priority:
        * Pattern matching
        * Macros for sum, lim, systems/des, etc.
        * Function composition
        * String interpolation
* Implement math tools
    * Research what algorithms you want to implement
    * Find efficient and pretty
* Plotting library (eg `plot(sin, 1, 5)`) plots sin b/w x=1, x=5
* Foreign function interface
* Visual debug tool
    * Not necessarily for stepping through code (lldb is fine)
    * Used for viewing the generated AST, bytecode, JIT details in a friendly
      manner.
    * Use web interface?
* Bytecode caching
    * Can hash file contents and store in `~/.tachyon/<hash>`?
    * Needs external hashing implementation?
* Tensors
* Runtime profiling program
    * This will help massively with showing how performance gains are found.
* Set up test data directory
* More tests and benchmarks
* Consider using mermaid over plantuml (github-flavoured markdown integraton)
* gcovr markdown generation (waiting on gcovr version 8.4 release)
* can strings be string_view?
* research more cache structures, such as 2Q, ARC, GDSF
* Support C++11?

## System Requirements

### Compilation

* C++ standard: C++23[^1]
* CMake version: 3.16

[^1]: I'm looking to phase away usage of `std::expected`, in order to decrease the minimum C++ standard version.

### Runtime

* Operating System: Linux
* Computer Architecture: x86_64

## Building

For release builds, with tests and benchmarks disabled:

```shell
cmake -S . -B cmake-build-release -G Ninja -D TACHYON_BUILD_TESTS=OFF -D TACHYON_BUILD_BENCHMARKS=OFF
cmake --build cmake-build-release
./cmake-build-release/tachyon run examples/01-quadratic-formula.tachyon
```

For debug builds:

```shell
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug
./cmake-build-debug/tachyon run examples/02-taylor-trigonometry.tachyon
```