# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Python Wheel Building (macOS)

### CRITICAL: Never Remove -undefined dynamic_lookup

**IMPORTANT**: When building Python wheels on macOS, NEVER suggest removing or replacing the `-Wl,-undefined,dynamic_lookup` linker flag.

**Why this flag is required:**
- Python wheels must be **portable** across different Python installations
- The Python interpreter used at build time may differ from runtime
- `-undefined dynamic_lookup` allows Python symbols to be resolved at runtime from whatever Python interpreter loads the extension
- Linking against a specific Python library at build time would break portability

**Common mistake to avoid:**
```cmake
# ❌ WRONG - This breaks wheel portability
target_link_libraries(pylibvw PRIVATE Python::Python ...)

# ✅ CORRECT - Use dynamic lookup on macOS
if(APPLE)
  # Don't link Python library - use -undefined dynamic_lookup
  target_link_libraries(pylibvw PRIVATE Boost::python ...)
else()
  target_link_libraries(pylibvw PRIVATE Python::Module ...)
endif()
```

**Symbol resolution issues with static libraries:**

If you encounter "symbol not found in flat namespace" errors when linking to static libraries (like vw_core), the solution is **NOT** to remove `-undefined dynamic_lookup`. Instead:

1. **Build the static library as SHARED on macOS for Python builds**
   - This prevents static object code from being incorporated into the MODULE
   - Shared library symbols resolve properly with `-undefined dynamic_lookup`
   - Example: Build vw_core as SHARED when BUILD_PYTHON=ON

2. **Never use `-flat_namespace`**
   - This is deprecated on modern macOS
   - macOS prefers two-level namespace

**Correct approach in vowpal_wabbit:**
```cmake
# vowpalwabbit/core/CMakeLists.txt
if(APPLE AND BUILD_PYTHON)
  # Build as SHARED for Python compatibility
  set(VW_CORE_LIB_TYPE "SHARED_ONLY")
else()
  set(VW_CORE_LIB_TYPE "STATIC_ONLY")
endif()

# python/CMakeLists.txt
if(APPLE)
  set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-undefined,dynamic_lookup" ...)
endif()
```

This ensures:
- Wheels remain portable (no specific Python version baked in)
- Symbol resolution works correctly
- delocate-wheel can properly bundle shared libraries
