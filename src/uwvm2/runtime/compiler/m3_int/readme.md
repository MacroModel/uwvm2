# M3 Interpreter

The **M3 Interpreter** is a C++ port of **`m3`** from **wasm3**. It is designed as an alternative interpreter backend for **uwvm2**.

## Background

wasm3 is widely recognized for being both **lightweight** and **high-performance**. Its compact design makes it especially attractive for environments where simplicity, portability, and low overhead matter, while still delivering strong interpreter performance.

Because of these qualities, we chose to port wasm3’s `m3` into C++ and integrate it as a candidate interpreter for uwvm2.

## Design

In the original wasm3 implementation, interpreter opcode handler functions are generated through **macros**.  
In this C++ port, that mechanism has been refactored to use **templates** instead.

This change provides several benefits:

- **Better type safety**
- **Improved readability and maintainability**
- **Cleaner abstraction for opcode generation**
- **Stronger integration with modern C++ codebases**

## Goal

The goal of the M3 Interpreter is to preserve the core advantages of wasm3:

- **small footprint**
- **efficient execution**
- **simple interpreter architecture**

while making the implementation better suited for the uwvm2 ecosystem through a more idiomatic C++ design.

## Position in uwvm2

Within uwvm2, the M3 Interpreter serves as a **backup / alternative interpreter implementation**.  
It offers a practical option when a lightweight interpreter with solid runtime performance is preferred.

## Summary

In short, the M3 Interpreter is:

- a **C++ port** of wasm3’s `m3`
- a **template-based redesign** of macro-generated opcode handlers
- a **lightweight and performant interpreter option** for uwvm2

It combines wasm3’s proven interpreter model with a C++-oriented implementation approach, making it a strong candidate as an alternative execution engine in uwvm2.
## Current Scope

The current port in `optable/` focuses on the opcode-handler families that map cleanly from wasm3’s macro-generated interpreter to C++26 templates:

- numeric unary/binary ops (`r`, `s`, `rs`, `sr`, `ss`), including floating `min` / `max` / `copysign`
- select op families (`rss`, `srs`, `ssr`, `sss`, plus floating `rrs` / `rsr`)
- convert/reinterpret families (`rr`, `rs`, `sr`, `ss`)
- slot/global helpers, branch/select control helpers, call/compile/loop/return-style control helpers, memory `load` / `store`, and bulk memory helpers (`MemSize`, `MemGrow`, `MemCopy`, `MemFill`)
- lightweight hook-based execution context plumbing for `Call`, `Compile`, `CallIndirect`, `CallRawFunction`, `Entry`, `Loop`, `Return`, `End`, `ContinueLoop`, and `ContinueLoopIf`
- trap hooks for unreachable, unsupported opcodes, divide-by-zero, integer overflow, invalid integer conversion, and out-of-bounds memory access
- opcode shape dispatch is now expressed with C++ templates only; the transplanted `optable/` no longer relies on local macro generators for these op families
- a new header-level straight-line translator scaffold in `straight_line.h` can now compile and execute a focused subset of Wasm expressions (`const`, `local.get/set/tee`, `drop`, `select`, direct `call`, `call_indirect`, `end`/`return`, and a broader MVP numeric subset covering comparisons, div/rem, bitwise/shift/rotate, rounding, sqrt, min/max, copysign, and core MVP convert/reinterpret operators), plus an initial structured-control subset (`block`, `loop`, `if`/`else`, `br`, `br_if`, `br_table`) with empty or single-value MVP result block types; when given module-function, type, and table references, it can lazily compile direct/indirect callees through the transplanted m3 call hooks for targeted tests and runtime bridging

`xmake` also exposes an `enable-int=m3int` configuration value for the transplanted backend naming. In addition to the transplanted optables and straight-line translator scaffold, `m3int` now has a minimal uwvm runtime bridge for `() -> ()` entry functions: it resolves the selected runtime entry, translates supported straight-line function bodies into m3-style threaded bytecode, and executes them through the transplanted interpreter backend.
