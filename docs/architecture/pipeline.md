# Pipeline

## Description

This document describes the overall architecture of the Tachyon programming language and
just-in-time compiler.

## Definitions

### <type of> Pipeline

A <type of> Pipeline is a sequence of steps that results in the manipulation of data.

These definitions are intended for the purposes of this document only.

### Bytecode

### Compile-time

Compile-time is any period in time during which the Tachyon program is attempting to translate a
given piece of source text to bytecode. This primarily includes the lexing, parsing, and bytecode
generation steps.

### Just-In-Time (JIT) Compilation

Just-In-Time compilation is the concept of compiling a specific set of instructions into machine
code just before running it. The specific instructions to compile are identified by the virtual
machine.

### Runtime

Runtime is any period in time during which the Tachyon program is attempting to execute bytecode
or its machine code counterpart. This primarily includes the virtual machine execution,
intermediate representation generation, and intermediate representation optimization steps.

### User

The user is anyone that is writing and expects to execute Tachyon code.

## Architecture

![pipeline.png or pipeline.puml](./pipeline.png)

The overall architecture of Tachyon is split into two key pipelines:

### Compile-time Pipeline

The compile-time pipeline consists of:

* Lexer (done): turns the source text into a list of tokens
* Parser (done): turns the list of tokens into an abstract syntax tree
* Bytecode Generator (done): turns the abstract syntax tree into a list of bytecode instructions

By the end of this pipeline, Tachyon has a list of bytecode instructions that represent the code
written by the user

### Runtime Pipeline

The runtime pipeline consists of:

* Tachyon Virtual Machine (incomplete): interprets the bytecode (done), executes machine code
  (incomplete), tracks/marks instructions for optimization (incomplete)
* Intermediate Representation Generator (incomplete): generates intermediate representation from
  bytecode

### Intermediate Representation

Why take the step of generating intermediate representation, when one could directly attempt to
optimize the bytecode (or ast)? The reason is that optimizing these structures is extremely
difficult and, even if implemented correctly, compute-intensive. It is easier to generate a
representation of the intent of the code (i.e. abstract individual instructions into an
intermediate representation) than to optimize the instructions directly.

### Marking Instructions for Optimization (TODO)