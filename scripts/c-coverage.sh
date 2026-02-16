#!/usr/bin/env bash
# Generate LLVM source-based coverage for libdda C code.
# Requires: CMake, Apple clang (or LLVM clang), xcrun llvm-profdata, xcrun llvm-cov
#
# Usage: bash scripts/c-coverage.sh
# Output: coverage-report/c/index.html (HTML) + text summary on stdout

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build-coverage"
REPORT_DIR="$REPO_ROOT/coverage-report/c"
PROFDATA_DIR="$BUILD_DIR/profdata"

# Tests that pass reliably (skip analysisdata_test and bolttest which have pre-existing failures)
TESTS=(
  constants_test
  geometrydata_test
  ddadlist_test
  inpoly_test
  material_test
  stress_test
  loadpointtest
  matmulttest
  all_tests
)

echo "=== C Coverage: configure ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake "$REPO_ROOT" -DENABLE_COVERAGE=ON -DCMAKE_C_COMPILER=clang 2>&1 | tail -3

echo "=== C Coverage: build ==="
cmake --build . -j"$(sysctl -n hw.ncpu)" 2>&1 | tail -3

echo "=== C Coverage: run tests ==="
rm -rf "$PROFDATA_DIR"
mkdir -p "$PROFDATA_DIR"

BINARIES=()
for test in "${TESTS[@]}"; do
  binary="$BUILD_DIR/tests/unit/$test"
  if [ ! -x "$binary" ]; then
    echo "  SKIP $test (binary not found)"
    continue
  fi
  echo -n "  $test ... "
  LLVM_PROFILE_FILE="$PROFDATA_DIR/${test}-%p.profraw" "$binary" > /dev/null 2>&1 && echo "ok" || echo "FAIL (ignored)"
  BINARIES+=("$binary")
done

echo "=== C Coverage: merge profiles ==="
PROFRAW_FILES=("$PROFDATA_DIR"/*.profraw)
if [ ${#PROFRAW_FILES[@]} -eq 0 ]; then
  echo "ERROR: No .profraw files generated"
  exit 1
fi

MERGED="$PROFDATA_DIR/merged.profdata"
xcrun llvm-profdata merge -sparse "${PROFRAW_FILES[@]}" -o "$MERGED"
echo "  Merged ${#PROFRAW_FILES[@]} profile(s)"

# Build the -object flags for llvm-cov (one per test binary)
OBJECT_FLAGS=()
for bin in "${BINARIES[@]}"; do
  OBJECT_FLAGS+=("-object" "$bin")
done

echo "=== C Coverage: generate report ==="
rm -rf "$REPORT_DIR"
mkdir -p "$REPORT_DIR"

# HTML report
xcrun llvm-cov show \
  "${OBJECT_FLAGS[@]}" \
  -instr-profile="$MERGED" \
  -format=html \
  -output-dir="$REPORT_DIR" \
  -ignore-filename-regex='platform/(win32|stubs)|tauri_stubs|probe\.c' \
  -ignore-filename-regex='tests/unit/'

# Text summary
echo ""
echo "=== Coverage Summary ==="
xcrun llvm-cov report \
  "${OBJECT_FLAGS[@]}" \
  -instr-profile="$MERGED" \
  -ignore-filename-regex='platform/(win32|stubs)|tauri_stubs|probe\.c' \
  -ignore-filename-regex='tests/unit/'

echo ""
echo "HTML report: file://$REPORT_DIR/index.html"
