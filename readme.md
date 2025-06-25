# Tachyon: a JIT maths-optimized language

Tachyon is a just-in-time compiled programming language optimized for
mathematical operations. It serves as a personal project, and includes a virtual
machine, bytecode, high-level intermediate representation, and abstract syntax
tree. It will employ various optimization techniques such
as [tracing just-in-time compilation](https://en.wikipedia.org/wiki/Tracing_just-in-time_compilation)
to create fast and efficient programs during runtime.

The next steps for this project are, in no particular order:

* Complete grammar
    * High priority:
        * Extended conditionals:
            * Else/else-if
            * Continue/break
    * Medium priority:
        * Arrays
        * Enums
        * Structs
        * Imports
        * return semantics (missing -> RETV, ending expr -> RETC/RETR)
    * Low priority:
        * Pattern matching
        * Macros for sum, lim, systems/des, etc.
        * Function composition
        * String interpolation
* Implement math tools
    * Research what algorithms you want to implement
    * Find efficient and pretty
* Plotting library (eg `plot(sin, 1, 5)`) plots sin b/w x=1, x=5
* Visual debug tool
    * Not necessarily for stepping through code (lldb is fine)
    * Used for viewing the generated AST, bytecode, JIT details in a friendly
      manner.
    * Use web interface?
* Bytecode caching
    * Can hash file contents and store in `~/.tachyon/<hash>`?
    * Needs external hashing implementation?
* Tracing just in time compilation
    * ~highest priority
* Research implementation of matrices not as 2D arrays
    * C++23 `std::mdspan`, `std::submdspan` not released.
* Runtime profiling program
    * This will help massively with showing how performance gains are found.
* Set up test data directory
* More tests and benchmarks
* Consider using mermaid over plantuml (github-flavoured markdown integraton)
* gcovr markdown generation (waiting on gcovr version 8.4 release)
* can strings be string_view?
* research more cache structures, such as 2Q, ARC, GDSF

## Building

For release builds, with tests and benchmarks disabled:

```shell
cmake -S . -B cmake-build-release -G Ninja -D TACHYON_BUILD_TESTS=OFF -D TACHYON_BUILD_BENCHMARKS=OFF
cmake --build cmake-build-release
./cmake-build-release/tachyon run examples/quadratic-formula.tachyon
```

For debug builds:

```shell
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug
./cmake-build-debug/tachyon run examples/memoization.tachyon
```