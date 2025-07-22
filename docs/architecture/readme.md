# Architecture documentation

In this directory, I outline a few key "architectural" ideas around Tachyon. Some of these documents
are meant as explanations to the concepts that are key to Tachyon, like `pipeline/readme.md`). Some
are meant as a sort of walk-through of how I tackled some problem in Tachyon (such as
`optimizations/lexer.md`). I hope that these documents will give the reader a better understanding
of what has been going through my mind as I write this code.

In addition, code itself is documented inline, so you can find some unique insights into
implementation details there, too.

## Definitions

These definitions aim to disambiguate the concepts mentioned in other documents in this directory.
The definitions here are intended for the purposes of Tachyon only.

<!-- definitions are sorted in alphabetical order -->

### <type of> Pipeline

A <type of> Pipeline is a sequence of steps in Tachyon that results in the manipulation of data
with the goal of eventually executing user intent.

### Bytecode

Bytecode is an instruction of virtual machine instructions based on a given instruction set.
Bytecode is a lower-level representation of a Tachyon program's intent.

### Compile-time

Compile-time is any period in time during which the Tachyon program is attempting to translate a
given piece of source text to bytecode. This primarily includes the lexing, parsing, and bytecode
generation steps.

### Just-In-Time (JIT) Compilation

Just-In-Time compilation is the concept of compiling a specific set of instructions into machine
code just before running it. The specific instructions to compile are identified by the virtual
machine.

### Intent

Simply put: "what the user wants the program to do".

### Memoization

[Memoization](https://en.wikipedia.org/wiki/Memoization) is the concept of storing the results
of previously called functions in memory, such that calling the same function with the same
arguments can just refer to the cached value, instead of executing the function again. Function
memoization only works on pure functions, so side effects are not "skipped" when the value is
retrieved from cache.

### Pure Function

A pure function is any function that always returns the same value given the same arguments, and
has no side effects (such as writing to files, printing to a console, etc.). If a function is
pure, it can be memoized, which involves its return value being cached and read in future calls
with the same arguments. An impure function cannot be cached this way because it will "skip"
these side effect operations (such as writing the output of a function to a file) during cache hits.

### Runtime

Runtime is any period in time during which the Tachyon program is attempting to execute bytecode
or its machine code counterpart. This primarily includes the virtual machine execution,
intermediate representation generation, and intermediate representation optimization steps.

### User

The user is anyone that is writing and expects to execute Tachyon code.

### Virtual Machine

The virtual machine is a JIT bytecode interpreter that executes the intent of a Tachyon program
based off of its given instructions.
