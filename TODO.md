# ntdda TODO

Remaining work after restructure and CMake addition.

## Restructure (future)

- [ ] Move lib sources into `src/lib/` subdirs: `analysis/`, `geometry/`, `contact/`, `solver/`
- [ ] Create `include/dda/` for public API headers; update includes
- [ ] Add `src/apps/console/` for console application entry points
- [ ] Remove legacy Makefiles once CMake is fully adopted and tested

## Build & tests

- [ ] Fix failing unit tests: constants_test, bolttest, stress_test, analysisdata_test
- [ ] Fix contacttest target (broken, won't link)
- [ ] Verify Win32 GUI build still works after `win32gui` → `platform/win32` move
- [ ] Add `include/.!*!*.rc` or similar to `.gitignore` for temp files

## Code quality

- [ ] Fix graphics.c array bounds warning (R[9], G[9], B[9] past end of float[9])
- [ ] Fix functions.c / functions.h massmatrix_linear array parameter mismatch
- [ ] Fix ddadlist.c deprecated non-prototype function definitions
- [ ] Fix ddamemory.c pointer-to-int cast warnings

## Documentation

- [ ] Update README with CMake build instructions
- [ ] Document platform/stubs vs platform/win32 layout
