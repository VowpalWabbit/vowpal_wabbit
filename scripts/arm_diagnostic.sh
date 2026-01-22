#!/bin/bash
# ARM64 Illegal Instruction Diagnostic Script
# This script helps identify the source of illegal instruction faults on ARM64 systems.
#
# Usage: ./scripts/arm_diagnostic.sh [path_to_binary_or_library]
#
# The script checks for:
# 1. CPU features available on the current system
# 2. LSE (Large System Extensions) atomic instructions in binaries
# 3. SVE (Scalable Vector Extension) instructions in binaries
# 4. Build flags used in compiled binaries

set -e

echo "========================================"
echo "ARM64 Illegal Instruction Diagnostic"
echo "========================================"
echo ""

# Check if we're on ARM64
ARCH=$(uname -m)
if [[ "$ARCH" != "aarch64" && "$ARCH" != "arm64" ]]; then
    echo "WARNING: This script is designed for ARM64 systems (current: $ARCH)"
    echo ""
fi

echo "=== System Information ==="
echo "Architecture: $ARCH"
echo "Kernel: $(uname -r)"
echo ""

echo "=== CPU Information ==="
if [ -f /proc/cpuinfo ]; then
    echo "CPU implementer: $(grep -m1 'CPU implementer' /proc/cpuinfo | cut -d: -f2 | xargs || echo 'unknown')"
    echo "CPU architecture: $(grep -m1 'CPU architecture' /proc/cpuinfo | cut -d: -f2 | xargs || echo 'unknown')"
    echo "CPU variant: $(grep -m1 'CPU variant' /proc/cpuinfo | cut -d: -f2 | xargs || echo 'unknown')"
    echo "CPU part: $(grep -m1 'CPU part' /proc/cpuinfo | cut -d: -f2 | xargs || echo 'unknown')"
    echo ""
    echo "CPU Features:"
    grep -m1 'Features' /proc/cpuinfo | cut -d: -f2 | tr ' ' '\n' | grep -v '^$' | sort | head -30
else
    echo "Cannot read /proc/cpuinfo"
fi
echo ""

# Key ARM64 features to check for
echo "=== Key Feature Support ==="
FEATURES=$(grep -m1 'Features' /proc/cpuinfo 2>/dev/null | cut -d: -f2 || echo "")

check_feature() {
    local feature=$1
    local desc=$2
    if echo "$FEATURES" | grep -qw "$feature"; then
        echo "  [YES] $feature - $desc"
    else
        echo "  [NO]  $feature - $desc"
    fi
}

check_feature "atomics" "LSE atomics (ARMv8.1+)"
check_feature "lrcpc" "Load-Acquire RCpc (ARMv8.3+)"
check_feature "sve" "Scalable Vector Extension"
check_feature "sve2" "SVE2"
check_feature "fp" "Floating point"
check_feature "asimd" "NEON/Advanced SIMD"
check_feature "aes" "AES instructions"
check_feature "sha1" "SHA1 instructions"
check_feature "sha2" "SHA2 instructions"
check_feature "crc32" "CRC32 instructions"
echo ""

# Function to check a binary for problematic instructions
check_binary() {
    local binary=$1
    echo "=== Analyzing: $binary ==="

    if [ ! -f "$binary" ]; then
        echo "  File not found: $binary"
        return 1
    fi

    echo "  File type: $(file -b "$binary")"

    # Check ELF build attributes
    if command -v readelf &> /dev/null; then
        echo ""
        echo "  Build attributes:"
        readelf -A "$binary" 2>/dev/null | grep -E "(Tag_|Build|Architecture)" | head -10 || echo "  (none found)"

        echo ""
        echo "  Build comment:"
        readelf -p .comment "$binary" 2>/dev/null | grep -v "^$" | head -5 || echo "  (none found)"
    fi

    if command -v objdump &> /dev/null; then
        echo ""
        echo "  LSE atomic instructions (ldadd, stadd, swp, cas, ldclr, ldset, ldeor, ldapr):"
        local lse_count=$(objdump -d "$binary" 2>/dev/null | grep -cE '\s(ldadd|stadd|swp|cas|ldclr|ldset|ldeor|ldapr)' || echo "0")
        echo "  Count: $lse_count"
        if [ "$lse_count" -gt 0 ]; then
            echo "  Examples:"
            objdump -d "$binary" 2>/dev/null | grep -E '\s(ldadd|stadd|swp|cas|ldclr|ldset|ldeor|ldapr)' | head -5
        fi

        echo ""
        echo "  SVE instructions (z0-z31, p0-p15 registers):"
        local sve_count=$(objdump -d "$binary" 2>/dev/null | grep -cE '\s(z[0-9]+|p[0-9]+)\.' || echo "0")
        echo "  Count: $sve_count"
        if [ "$sve_count" -gt 0 ]; then
            echo "  Examples:"
            objdump -d "$binary" 2>/dev/null | grep -E '\s(z[0-9]+|p[0-9]+)\.' | head -5
        fi
    fi

    echo ""
}

# Check specified binary or default locations
if [ -n "$1" ]; then
    check_binary "$1"
else
    echo "=== Checking System Boost Libraries ==="
    echo ""

    # Find and check Boost libraries
    boost_libs=$(find /usr -name "libboost*.so*" 2>/dev/null | head -10)
    if [ -z "$boost_libs" ]; then
        echo "No Boost libraries found in /usr"
    else
        for lib in $boost_libs; do
            check_binary "$lib"
        done
    fi

    # Check for VW binaries
    if [ -f "./build/vowpalwabbit/cli/vw" ]; then
        check_binary "./build/vowpalwabbit/cli/vw"
    fi

    # Check for Python module
    python_lib=$(python3 -c "import sysconfig; print(sysconfig.get_path('platlib'))" 2>/dev/null || echo "")
    if [ -n "$python_lib" ]; then
        pylibvw=$(find "$python_lib" -name "pylibvw*.so" 2>/dev/null | head -1)
        if [ -n "$pylibvw" ]; then
            check_binary "$pylibvw"
        fi
    fi
fi

echo "=== Diagnostic Summary ==="
echo ""
echo "If illegal instruction errors occur on this system, check:"
echo "1. Whether the CPU supports 'atomics' feature (LSE)"
echo "2. Whether linked libraries contain LSE/SVE instructions"
echo "3. Build flags used (-march, -mno-outline-atomics)"
echo ""
echo "Recommendations for baseline ARMv8.0 compatibility:"
echo "  - Use -march=armv8-a (not armv8.1-a or higher)"
echo "  - Use -mno-outline-atomics to disable LSE"
echo "  - Use -mtune=generic"
echo "  - Ensure all linked libraries are built the same way"
echo ""
