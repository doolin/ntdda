/*
 * Tauri-specific stubs for symbols referenced by libdda but defined
 * in Win32 code. Parallel to platform/stubs/stubs.c.
 */
#include <stdlib.h>
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

/* _putenv — Windows API used by dda_set_output_directory in dda.c.
 * On POSIX, delegate to putenv().
 */
int _putenv(const char *envstring) {
    if (!envstring) return -1;
    /* putenv requires a non-const, persistent string */
    char *copy = strdup(envstring);
    if (!copy) return -1;
    return putenv(copy);
}
