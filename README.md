# ntdda

**ntdda** is a Windows C application for **Discontinuous Deformation Analysis (DDA)**—a numerical method for modeling blocky rock masses, joints, contacts, stress, and deformation in geotechnical and rock mechanics applications.

---

## Repository Structure

- **~209** C source and header files
- **`include/`** – Headers for analysis, geometry, contacts, stress, materials, bolts, joints, DXF import, XML parsing
- **`resources/`** – Icons, bitmaps (earthquake, rockbolt, geometry, etc.)
- **`src/`** – Numerical core; `platform/stubs` (non-Win32), `platform/win32` (Win32 GUI)
- **`tests/unit/`** – Unit tests
- **`src/Makefile`** – Builds `libdda.a` (legacy, kept)
- **`tests/unit/Makefile`** – Unit tests (legacy, kept)
- **`CMakeLists.txt`** – CMake build (recommended)
- **`winclean.bat`** – Windows build helper

---

## Building

### CMake (recommended)

```bash
cmake -B build
cmake --build build
```

Produces `libdda.a` and unit test binaries. Requires libxml2.

### Legacy Makefiles

```bash
cd src && make          # build libdda.a
cd tests/unit && make   # build unit tests
make test               # run all tests
```

---

## Testing

### C Unit Tests (CTest)

Run all C tests from the build directory:

```bash
cmake -B build
cmake --build build
ctest --test-dir build
```

Individual tests can also be run directly:

```bash
./build/tests/unit/constants_test
./build/tests/unit/geometrydata_test
./build/tests/unit/inpoly_test
# etc.
```

Tests: constants, geometrydata, analysisdata, bolttest, ddadlist, inpoly, material, stress, loadpoint, matmult, all_tests.

Note: `analysisdata_test` and `bolttest` have pre-existing failures.

### C Integration Test (pipeline_test)

Exercises the full DDA pipeline: XML parsing, ddacut, analysis loading, ddanalysis, and file output.
Requires geometry and analysis fixture files as arguments:

```bash
./build/tests/unit/pipeline_test \
  tests/fixtures/pushblock.geo \
  tests/fixtures/pushblock.ana
```

CTest runs this automatically with the correct paths. Output artifacts are written to an `output/` subdirectory next to the geometry file (e.g., `tests/fixtures/output/`).

Test fixtures in `tests/fixtures/` are private test data, separate from `examples/` which ships with the product. Generated output files in `tests/fixtures/output/` are git-ignored.

### Rust Tests (Tauri)

```bash
cd src/platform/tauri/src-tauri
cargo test
```

Tests: struct layout validation (geometrydata, analysisdata, filepaths) and pushblock full-pipeline integration test.

### Test Coverage (C)

Uses CMake with LLVM source-based coverage (`-fprofile-instr-generate -fcoverage-mapping`).
Requires Apple clang and Xcode command line tools (`xcrun llvm-profdata`, `xcrun llvm-cov`).

```bash
bash scripts/c-coverage.sh
```

Builds in `build-coverage/`, runs 10 tests (9 unit + pipeline integration), generates report at `coverage-report/c/index.html`.

### Test Coverage (Rust)

Requires `cargo-llvm-cov` and Homebrew LLVM:

```bash
cargo install cargo-llvm-cov

cd src/platform/tauri/src-tauri
LLVM_COV=/opt/homebrew/opt/llvm/bin/llvm-cov \
LLVM_PROFDATA=/opt/homebrew/opt/llvm/bin/llvm-profdata \
cargo llvm-cov --html --output-dir ../../../../coverage-report/html
```

Open the report: `coverage-report/html/index.html`

Text summary (no HTML):
```bash
LLVM_COV=/opt/homebrew/opt/llvm/bin/llvm-cov \
LLVM_PROFDATA=/opt/homebrew/opt/llvm/bin/llvm-profdata \
cargo llvm-cov
```

---

## Technical Details

- **Platform:** Win32 GUI (menus, dialogs, toolbar, status bar)
- **Features:** Static/dynamic analysis, Mohr-Coulomb materials, rockbolts, joints, tunnels, DXF import, XML geometry/analysis files
- **References:** Gen-Hua Shi's DDA program; LAPACK for linear algebra
- **Analysis:** Blocks, contacts, time steps, iterations, fixed/load/measured points

---

## Commit History (247 commits)

| Period | Activity |
|--------|----------|
| **May 2001** | Initial revision, coordinate tracking for "Mission Peak" runout |
| **2002–2003** | DXF import, XML parsing, Roozbeh's contributions (joints, Mohr-Coulomb, dialogs, zoom, toolbar) |
| **~2005** | VS2005 project files, time-step fix, time info dialog |
| **Long gap** | Little or no activity |
| **~2010s** | Revival: "First compile and run on some of this code since 2002. Still works. C is such a cool programming language." |
| **Recent** | Whitespace/formatting cleanup, bolt allocation refactor, unit tests, compile warning fixes |

---

## Contributors

- **doolin** – Primary author
- **Roozbeh** – Mohr-Coulomb, joint drawing, geometry dialogs, zoom, toolbar buttons
