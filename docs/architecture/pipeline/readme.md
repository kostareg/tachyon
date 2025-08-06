# Pipeline

## Description

This document describes the overall architecture of the Tachyon programming language and
just-in-time compiler.

## Architecture

![pipeline.png or pipeline.puml](./pipeline.png)

The overall architecture of Tachyon is split into two key pipelines:

### Compile-time Pipeline

The compile-time pipeline consists of:

* Lexer: turns the source text into a list of tokens
* Parser: turns the list of tokens into an abstract syntax tree
* Bytecode Generator: turns the abstract syntax tree into a list of bytecode instructions

By the end of this pipeline, Tachyon has a list of bytecode instructions that represent the code
written by the user

### Runtime Pipeline

The runtime pipeline consists of:

* Tachyon Virtual Machine: interprets the bytecode, executes machine code, tracks/marks functions
  for optimization
* Intermediate Representation Generator: generates intermediate representation from bytecode
* Intermediate Representation Optimizer (incomplete): applies various optimizations to intermediate
  representations
* Machine Code Generator: generates machine code from the intermediate representation

Once the machine code is ready, it is passed to the Tachyon Virtual Machine which executes it.

### Intermediate Representation

Why take the step of generating intermediate representation, when one could directly attempt to
optimize the bytecode (or ast)? The reason is that optimizing these structures is extremely
difficult and, even if implemented correctly, compute-intensive. It is easier to generate a
representation of the intent of the code (i.e. abstract individual instructions into an
intermediate representation) than to optimize the instructions directly.

### Marking Functions for Memoization

During bytecode generation, the kinds of instructions are recorded. If all instructions are pure,
the function overall is marked as pure, and the virtual machine is able to cache its input and
return values during runtime. Function impurities propagates to parents. If function a calls
function b that is impure, a is implicitly impure.

### Marking Instructions for Optimization (TODO)