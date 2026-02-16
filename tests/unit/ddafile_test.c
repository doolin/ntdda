/*
 * ddafile_test.c — White-box tests for DDAFile utility functions.
 *
 * Targets three functions in ddafile.c (lines 37-66) that have 0%
 * coverage and contain known bugs:
 *
 * Bug 1 (ddafile_new): Ignores the filename parameter — never copies
 *   it into df->filename.  Uses malloc (not calloc), so filename
 *   contains whatever the allocator left behind.
 *
 * Bug 2 (ddafile_new_open): Calls fopen(df->filename, "w") with the
 *   uninitialized garbage from Bug 1.  The file it creates (if any)
 *   has a random name.
 *
 * Bug 3 (ddafile_open_file): Uses sizeof(rootname) which evaluates to
 *   sizeof(char*) = 8 on 64-bit, not the string length.  Also calls
 *   strncpy twice from temp[0] instead of concatenating, so the
 *   second call overwrites the first.
 *
 * Uses the 0xDA/0xDD poison pattern (same as dda_new/dda_delete) to
 * make uninitialized and freed memory obvious in the output.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "ddafile.h"

/* Global required by loghtml.c, replay.c, etc. in libdda */
Filepaths filepath;

/*
 * Complete the opaque DDAFile type for white-box inspection.
 * Must match struct _ddafile in ddafile.c exactly:
 *
 *   struct _ddafile {
 *      char filename[1024];
 *      FILE * fp;
 *   };
 */
struct _ddafile {
    char filename[1024];
    FILE *fp;
};

/* These functions exist in ddafile.c but are NOT declared in the
 * public header ddafile.h.  Declare them here so we can call them.
 */
DDAFile *ddafile_new(const char *filename);
DDAFile *ddafile_new_open(const char *filename);
void     ddafile_open_file(FILE **fp, const char *rootname,
                           const char *filename);

#define DDAFILE_SIZE  sizeof(struct _ddafile)

/*
 * Pre-poison: allocate a block, fill with 0xDA, free it.
 * Many allocators will hand back this same block on the next
 * malloc of the same size, making uninitialized reads obvious:
 * you see 0xDADADADA instead of whatever happened to be there.
 */
static void poison_next_malloc(size_t size)
{
    void *p = malloc(size);
    if (p) {
        memset(p, 0xDA, size);
        free(p);
    }
}

static int file_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

/* ------------------------------------------------------------------ */

int main(void)
{
    int passed = 1;

    fprintf(stderr, "=== ddafile_test ===\n");

    /* Verify our struct mirror matches the real size.
     * If this fails, our white-box inspection is invalid.
     */
    {
        DDAFile *probe = ddafile_new("probe");
        /* DDAFile* is opaque but we know malloc(sizeof(DDAFile)) was called.
         * We can't directly assert sizeof equality without the real struct,
         * but DDAFILE_SIZE = 1024 + sizeof(FILE*) should be 1032 on LP64.
         */
        fprintf(stderr, "  DDAFile mirror size: %zu bytes\n", DDAFILE_SIZE);
        memset(probe, 0xDD, DDAFILE_SIZE);
        free(probe);
    }


    /* ---- Test 1: ddafile_new ignores filename parameter ---- */
    fprintf(stderr, "\n--- Test 1: ddafile_new ignores filename ---\n");
    {
        poison_next_malloc(DDAFILE_SIZE);

        DDAFile *df = ddafile_new("test_should_be_copied.txt");
        assert(df != NULL);

        /* Bug: filename was NOT copied.  Field contains whatever malloc
         * returned — likely 0xDA from our poison, or other garbage.
         */
        unsigned char first = (unsigned char)df->filename[0];
        fprintf(stderr, "  filename[0] = 0x%02X", first);

        if (first == 0xDA)
            fprintf(stderr, " (0xDA poison — uninitialized!)");
        else if (first == 0x00)
            fprintf(stderr, " (zero — calloc or lucky)");
        else
            fprintf(stderr, " (heap garbage)");

        if (strcmp(df->filename, "test_should_be_copied.txt") == 0) {
            fprintf(stderr, "\n  UNEXPECTED: filename was copied\n");
            passed = 0;
        } else {
            fprintf(stderr, "\n  BUG CONFIRMED: filename parameter ignored\n");
        }

        /* The one thing ddafile_new DOES set correctly */
        assert(df->fp == NULL);
        fprintf(stderr, "  fp = NULL (correctly initialized)\n");

        /* Post-poison with 0xDD before free (mirrors dda_delete pattern) */
        memset(df, 0xDD, DDAFILE_SIZE);
        free(df);
    }


    /* ---- Test 2: ddafile_new_open fopens garbage filename ---- */
    fprintf(stderr, "\n--- Test 2: ddafile_new_open uses garbage filename ---\n");
    {
        unlink("test_new_open.txt");

        poison_next_malloc(DDAFILE_SIZE);

        DDAFile *df = ddafile_new_open("test_new_open.txt");
        assert(df != NULL);

        /* Bug: fopen was called with garbage filename, NOT "test_new_open.txt".
         * The intended file was never created.
         */
        if (!file_exists("test_new_open.txt")) {
            fprintf(stderr, "  BUG CONFIRMED: \"test_new_open.txt\" was NOT created\n");
            fprintf(stderr, "  fopen used garbage from uninitialized df->filename\n");
        } else {
            fprintf(stderr, "  NOTE: file exists (allocator coincidence)\n");
            unlink("test_new_open.txt");
        }

        if (df->fp == NULL) {
            fprintf(stderr, "  fp = NULL (fopen failed on garbage path)\n");
        } else {
            fprintf(stderr, "  fp = %p (fopen succeeded on garbage path!)\n",
                    (void *)df->fp);
            fclose(df->fp);
        }

        /* Show the garbage filename that fopen actually received */
        fprintf(stderr, "  actual filename bytes: "
                "%02X %02X %02X %02X %02X %02X %02X %02X ...\n",
                (unsigned char)df->filename[0],
                (unsigned char)df->filename[1],
                (unsigned char)df->filename[2],
                (unsigned char)df->filename[3],
                (unsigned char)df->filename[4],
                (unsigned char)df->filename[5],
                (unsigned char)df->filename[6],
                (unsigned char)df->filename[7]);

        memset(df, 0xDD, DDAFILE_SIZE);
        free(df);
    }


    /* ---- Test 3: ddafile_open_file sizeof(pointer) truncation ---- */
    fprintf(stderr, "\n--- Test 3: ddafile_open_file sizeof(pointer) bug ---\n");
    {
        FILE *fp = NULL;

        /* Both strings are deliberately longer than sizeof(char*) = 8
         * to expose the truncation.
         */
        const char *rootname = "/long/path/to/root";   /* 18 chars */
        const char *filename = "important_output.dat";  /* 20 chars */

        /*
         * What SHOULD happen:
         *   temp = "/long/path/to/root" + "important_output.dat"
         *   fopen("/long/path/to/rootimportant_output.dat", "w")
         *
         * What ACTUALLY happens (three bugs):
         *   1. strncpy(temp, rootname, sizeof(rootname))
         *      sizeof(rootname) = sizeof(char*) = 8
         *      temp = "/long/pa" (only 8 bytes copied)
         *
         *   2. strncpy(temp, filename, sizeof(filename))
         *      sizeof(filename) = sizeof(char*) = 8
         *      OVERWRITES from temp[0]: temp = "importan"
         *      (should have been strncat to concatenate)
         *
         *   3. char temp[1024] = {'0'} sets temp[0] to ASCII 0x30,
         *      not '\0'.  Harmless since strncpy overwrites it, but
         *      reveals a misunderstanding.
         *
         * Net result: fopen("importan", "w")
         */
        unlink("importan");

        ddafile_open_file(&fp, rootname, filename);

        if (fp != NULL) {
            fclose(fp);

            if (file_exists("importan")) {
                fprintf(stderr, "  BUG CONFIRMED: created \"importan\" instead of full path\n");
                fprintf(stderr, "  sizeof(char*) = %zu — only %zu bytes copied per strncpy\n",
                        sizeof(const char *), sizeof(const char *));
                fprintf(stderr, "  Second strncpy overwrote first (should be strncat)\n");
                unlink("importan");
            } else {
                fprintf(stderr, "  fopen succeeded but on unexpected filename\n");
            }
        } else {
            fprintf(stderr, "  fopen returned NULL (truncated path didn't resolve)\n");
            fprintf(stderr, "  sizeof(char*) = %zu — strncpy only copied %zu bytes\n",
                    sizeof(const char *), sizeof(const char *));
        }
    }


    /* ---- Test 4: ddafile_open_file with short strings ---- */
    fprintf(stderr, "\n--- Test 4: ddafile_open_file short strings (<= 8 chars) ---\n");
    {
        FILE *fp = NULL;

        /* With strings shorter than sizeof(char*), the truncation
         * is hidden.  But the overwrite bug is still present:
         * rootname is overwritten by filename.
         */
        const char *rootname = "root";   /* 4 chars */
        const char *filename = "name";   /* 4 chars */

        unlink("name");

        ddafile_open_file(&fp, rootname, filename);

        if (fp != NULL) {
            fclose(fp);

            /* The file should be named "rootname" if concatenated,
             * but it's just "name" because strncpy overwrites.
             */
            if (file_exists("name")) {
                fprintf(stderr, "  BUG CONFIRMED: created \"name\" not \"rootname\"\n");
                fprintf(stderr, "  strncpy overwrites instead of concatenating\n");
                unlink("name");
            } else if (file_exists("rootname")) {
                fprintf(stderr, "  UNEXPECTED: concatenation worked\n");
                passed = 0;
                unlink("rootname");
            }
        } else {
            fprintf(stderr, "  fopen returned NULL\n");
        }
    }


    fprintf(stderr, "\n=== ddafile_test: %s ===\n", passed ? "PASSED" : "FAILED");
    return passed ? 0 : 1;
}
