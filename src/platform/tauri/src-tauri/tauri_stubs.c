/*
 * Tauri-specific stubs for symbols referenced by libdda but defined
 * in Win32 code. Parallel to platform/stubs/stubs.c.
 */
#include <string.h>
#include "ddafile.h"

/* Global filepath variable — defined in winmain.c for Win32,
 * referenced by loghtml.c, replay.c, etc.
 */
Filepaths filepath;

/* initFilePaths — declared in ddafile.h but never implemented.
 * Zero-initialize all path buffers.
 */
void initFilePaths(Filepaths * fp) {
    if (fp) memset(fp, 0, sizeof(Filepaths));
}
