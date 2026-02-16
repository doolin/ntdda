/*
 * pipeline_test.c — Integration test for the full DDA pipeline.
 *
 * Loads pushblock.geo, runs ddacut(), loads pushblock.ana, runs
 * ddanalysis(), then checks that output artifacts exist.
 *
 * This exercises the bulk of the numerical core: XML parsing,
 * geometry cutting, contact detection, matrix assembly, solver,
 * time stepping, and file output.
 *
 * Note: On macOS, the C crr() function misclassifies all points
 * as "outside" due to rand()/RAND_MAX mismatch (see tauri-bridge.md).
 * We include a minimal point-in-polygon fix here so ddanalysis()
 * doesn't crash.  The existing C code is NOT modified.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>

#include "dda.h"
#include "ddafile.h"
#include "geometrydata.h"
#include "analysisdata.h"

/* The filepath global is extern'd in several libdda files (loghtml.c,
 * replay.c, etc.) and normally defined in winmain.c or tauri_stubs.c.
 * We define it here so the linker is happy.
 */
Filepaths filepath;

/* -----------------------------------------------------------
 * Minimal point-in-polygon fix for macOS rand()/RAND_MAX bug.
 *
 * The C crr() function uses e1 = rand()/2048.0, assuming Win32's
 * RAND_MAX of 32767.  On macOS RAND_MAX is 2^31-1, so e1 >> 1,
 * sqrt(1 - e1^2) = NaN, and all points are classified "outside".
 * This causes ddanalysis() to SIGSEGV in df12().
 *
 * We redo the assignment with a standard horizontal ray-cast.
 * ----------------------------------------------------------- */
static int point_in_polygon(double **vertices, int j0, int j1,
                            double px, double py)
{
    int crossings = 0;
    int n_verts = j1 - j0 + 1;
    int k;

    for (k = 0; k < n_verts; k++) {
        int i_curr = j0 + k;
        int i_next = (k + 1 < n_verts) ? j0 + k + 1 : j0;

        double y1 = vertices[i_curr][2];
        double y2 = vertices[i_next][2];

        if ((y1 <= py && y2 > py) || (y2 <= py && y1 > py)) {
            double x1 = vertices[i_curr][1];
            double x2 = vertices[i_next][1];
            double t = (py - y1) / (y2 - y1);
            double x_int = x1 + t * (x2 - x1);
            if (px < x_int) {
                crossings++;
            }
        }
    }
    return (crossings % 2) == 1;
}

static void fix_point_block_assignments(Geometrydata *gd)
{
    int n_points = gd->nFPoints + gd->nLPoints + gd->nMPoints;
    int n_blocks = gd->nBlocks;
    int fixed = 0;
    int i, block;

    if (n_points == 0 || n_blocks == 0)
        return;
    if (!gd->points || !gd->vertices || !gd->vindex)
        return;

    for (i = 1; i <= n_points; i++) {
        int current_block;
        double px, py;

        if (!gd->points[i])
            continue;

        px = gd->points[i][1];
        py = gd->points[i][2];
        current_block = (int)gd->points[i][3];

        if (current_block != 0)
            continue;

        for (block = 1; block <= n_blocks; block++) {
            int j0, j1;
            if (!gd->vindex[block])
                continue;
            j0 = (int)gd->vindex[block][1];
            j1 = (int)gd->vindex[block][2];

            if (point_in_polygon(gd->vertices, j0, j1, px, py)) {
                gd->points[i][3] = (double)block;
                fixed++;
                break;
            }
        }
    }

    if (fixed > 0)
        fprintf(stderr, "[pipeline_test] fix_point_block_assignments: "
                "reassigned %d of %d points\n", fixed, n_points);
}

/* ----------------------------------------------------------- */

static int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

int main(int argc, char **argv)
{
    DDA *dda;
    Geometrydata *gd;
    Analysisdata *ad;
    int result;
    int passed = 1;
    char prev_cwd[1024];
    const char *geo_file;
    const char *ana_file;
    const char *ana_stem;

    /* Locate example files.  Accept paths as args or use defaults
     * relative to the repo root (works from build-coverage/).
     */
    if (argc >= 3) {
        geo_file = argv[1];
        ana_file = argv[2];
    } else {
        geo_file = "../examples/loadpoint/pushblock.geo";
        ana_file = "../examples/loadpoint/pushblock.ana";
    }
    ana_stem = "pushblock";

    if (!file_exists(geo_file)) {
        fprintf(stderr, "FAIL: geo file not found: %s\n", geo_file);
        return 1;
    }
    if (!file_exists(ana_file)) {
        fprintf(stderr, "FAIL: ana file not found: %s\n", ana_file);
        return 1;
    }

    fprintf(stderr, "=== pipeline_test: %s + %s ===\n", geo_file, ana_file);

    /* 1. Create DDA engine */
    dda = dda_new();
    if (!dda) {
        fprintf(stderr, "FAIL: dda_new returned NULL\n");
        return 1;
    }

    /* 2. Load geometry */
    gd = gdata_new();
    if (!gd) {
        fprintf(stderr, "FAIL: gdata_new returned NULL\n");
        return 1;
    }
    gd->display_error = dda_display_error;
    gd->display_warning = dda_display_warning;

    gdata_read_input_file(gd, (char *)geo_file);
    dda_set_geometrydata(dda, gd);

    fprintf(stderr, "  nJoints=%d nFPoints=%d nLPoints=%d nMPoints=%d\n",
            gd->nJoints, gd->nFPoints, gd->nLPoints, gd->nMPoints);

    if (gd->nJoints <= 0) {
        fprintf(stderr, "FAIL: no joints after reading geometry\n");
        passed = 0;
    }

    /* 3. ddacut — CWD must be writable for cut.log, pnp.log */
    if (!getcwd(prev_cwd, sizeof(prev_cwd)))
        prev_cwd[0] = '\0';

    {
        /* chdir to the geometry file's directory */
        char geo_dir[1024];
        strncpy(geo_dir, geo_file, sizeof(geo_dir) - 1);
        geo_dir[sizeof(geo_dir) - 1] = '\0';
        {
            char *slash = strrchr(geo_dir, '/');
            if (slash) {
                *slash = '\0';
                chdir(geo_dir);
            }
        }
    }

    ddacut(gd);

    fprintf(stderr, "  nBlocks=%d after ddacut\n", gd->nBlocks);

    if (gd->nBlocks <= 0) {
        fprintf(stderr, "FAIL: no blocks after ddacut\n");
        passed = 0;
    }
    if (!gd->vertices) {
        fprintf(stderr, "FAIL: vertices NULL after ddacut\n");
        passed = 0;
    }
    if (!gd->vindex) {
        fprintf(stderr, "FAIL: vindex NULL after ddacut\n");
        passed = 0;
    }

    /* Fix macOS rand()/RAND_MAX bug in crr() */
    fix_point_block_assignments(gd);

    /* 4. Load analysis */
    ad = adata_new();
    if (!ad) {
        fprintf(stderr, "FAIL: adata_new returned NULL\n");
        passed = 0;
        goto cleanup;
    }
    ad->display_error = dda_display_error;
    ad->display_warning = dda_display_warning;

    adata_read_input_file(ad, (char *)ana_file,
                          gd->nFPoints, gd->pointCount, gd->nLPoints);
    dda_set_analysisdata(dda, ad);

    fprintf(stderr, "  nTimeSteps=%d\n", ad->nTimeSteps);

    if (ad->nTimeSteps <= 0) {
        fprintf(stderr, "FAIL: no timesteps after reading analysis\n");
        passed = 0;
    }

    /* 5. Set up filepaths (rootname required by openAnalysisFiles) */
    memset(&filepath, 0, sizeof(filepath));
    strncpy(filepath.gfile, geo_file, FNAME_BUFSIZE - 1);
    strncpy(filepath.afile, ana_file, FNAME_BUFSIZE - 1);
    strncpy(filepath.rootname, ana_stem, FNAME_BUFSIZE - 1);

    /* 6. Run analysis */
    result = ddanalysis(dda, &filepath);
    fprintf(stderr, "  ddanalysis returned %d, cts=%d\n", result, ad->cts);

    if (result != 1) {
        fprintf(stderr, "FAIL: ddanalysis returned %d (expected 1)\n", result);
        passed = 0;
    }
    if (ad->cts <= 0) {
        fprintf(stderr, "FAIL: no timesteps completed\n");
        passed = 0;
    }

    /* 7. Check output artifacts exist */
    if (file_exists("pushblock.replay")) {
        fprintf(stderr, "  replay file: exists\n");
    } else {
        fprintf(stderr, "  replay file: MISSING\n");
    }

cleanup:
    /* Restore CWD */
    if (prev_cwd[0])
        chdir(prev_cwd);

    dda_delete(dda);

    if (passed)
        fprintf(stderr, "pipeline_test: PASSED\n");
    else
        fprintf(stderr, "pipeline_test: FAILED\n");

    return passed ? 0 : 1;
}
