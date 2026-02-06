# VowpalWabbit .NET Bindings

This directory contains the .NET bindings for Vowpal Wabbit.

## Available Bindings

### .NET Standard Bindings (Recommended)

The **recommended** bindings are the cross-platform .NET Standard bindings located in:

- `vw.net/` - Core bindings (VowpalWabbit.Core)
- `cs/` - Main VowpalWabbit wrapper
- `cs_json/` - JSON serialization support
- `cs_parallel/` - Parallel processing support
- `common/` - Common utilities

These bindings:
- Target **.NET Standard 2.0**, compatible with:
  - .NET Framework 4.6.1+
  - .NET Core 2.0+
  - .NET 5/6/7/8+
- Work on **Windows, Linux, and macOS**
- Use P/Invoke to call the native VowpalWabbit library

### C++/CLI Bindings (Deprecated)

> **DEPRECATED**: The C++/CLI bindings in `cli/` are deprecated and will be removed in a future release.

The C++/CLI bindings:
- Are **Windows-only**
- Require the MSVC toolchain
- Only support .NET Framework

**Please migrate to the .NET Standard bindings above.**

## Migration Guide

If you are currently using the C++/CLI bindings (`VowpalWabbit.Core` from `cs/cli/`), migration to the .NET Standard bindings is straightforward:

1. Replace your NuGet package reference from the C++/CLI package to the .NET Standard package
2. The API is largely compatible - most code will work without changes
3. Ensure the native VowpalWabbit library (`vw.net.native`) is available at runtime

## Building

### .NET Standard Bindings

```bash
cd cs
dotnet build
```

### C++/CLI Bindings (Deprecated)

The C++/CLI bindings are built via CMake on Windows with the `/clr` flag.
