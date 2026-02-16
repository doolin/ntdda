/*
 * inpoly_test.c — Unit tests for point-in-polygon functions.
 *
 * Tests both inpoly() and pointinpoly() from inpoly.c.
 * Both implement ray-casting with identical logic but different
 * array conventions:
 *
 *   inpoly():       poly[1..npoints][1..2], standalone polygon
 *   pointinpoly():  vindex/vertices in DDA block format
 *
 * The unit square is defined once and adapted to both interfaces.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "inpoly.h"

/* ------------------------------------------------------------------ */
/* 1-indexed 2D array helpers                                         */
/* ------------------------------------------------------------------ */

static double **alloc_2d_double(int rows, int cols)
{
    double **arr = calloc(rows + 1, sizeof(double *));
    assert(arr != NULL);
    for (int i = 0; i <= rows; i++) {
        arr[i] = calloc(cols + 1, sizeof(double));
        assert(arr[i] != NULL);
    }
    return arr;
}

static void free_2d_double(double **arr, int rows)
{
    for (int i = 0; i <= rows; i++)
        free(arr[i]);
    free(arr);
}

static int **alloc_2d_int(int rows, int cols)
{
    int **arr = calloc(rows + 1, sizeof(int *));
    assert(arr != NULL);
    for (int i = 0; i <= rows; i++) {
        arr[i] = calloc(cols + 1, sizeof(int));
        assert(arr[i] != NULL);
    }
    return arr;
}

static void free_2d_int(int **arr, int rows)
{
    for (int i = 0; i <= rows; i++)
        free(arr[i]);
    free(arr);
}

/* ------------------------------------------------------------------ */
/* Unit square: (0,0) (1,0) (1,1) (0,1)                              */
/* ------------------------------------------------------------------ */

#define UNIT_SQ_NPOINTS 4

/* For inpoly(): poly[1..4][1..2] */
static double **make_unit_square_poly(void)
{
    double **poly = alloc_2d_double(UNIT_SQ_NPOINTS, 2);
    poly[1][1] = 0.0;  poly[1][2] = 0.0;
    poly[2][1] = 1.0;  poly[2][2] = 0.0;
    poly[3][1] = 1.0;  poly[3][2] = 1.0;
    poly[4][1] = 0.0;  poly[4][2] = 1.0;
    return poly;
}

/*
 * For pointinpoly(): DDA vindex/vertices layout.
 *
 * pointinpoly() computes:
 *   start = vindex[block][1] - 1
 *   stop  = vindex[block][2] + 1
 * and iterates vertices[start..stop], so we need one slot of
 * padding before the first vertex and a closing vertex after
 * the last unique vertex.
 *
 * Layout (block 1):
 *   vertices[0] = padding       (0,0)
 *   vertices[1] = first vertex  (0,0)
 *   vertices[2] =               (1,0)
 *   vertices[3] =               (1,1)
 *   vertices[4] = last unique   (0,1)
 *   vertices[5] = closing       (0,0) = vertices[1]
 *   vindex[1][1] = 1, vindex[1][2] = 4
 *   → start = 0, stop = 5
 */
#define UNIT_SQ_NVERTS 5  /* indices 0..5 */

static void make_unit_square_dda(int ***vindex_out, double ***vertices_out)
{
    int **vindex = alloc_2d_int(1, 2);
    double **vertices = alloc_2d_double(UNIT_SQ_NVERTS, 2);

    vindex[1][0] = 1;  /* material */
    vindex[1][1] = 1;  /* start vertex */
    vindex[1][2] = 4;  /* end unique vertex */

    vertices[0][1] = 0.0;  vertices[0][2] = 0.0;  /* padding */
    vertices[1][1] = 0.0;  vertices[1][2] = 0.0;
    vertices[2][1] = 1.0;  vertices[2][2] = 0.0;
    vertices[3][1] = 1.0;  vertices[3][2] = 1.0;
    vertices[4][1] = 0.0;  vertices[4][2] = 1.0;
    vertices[5][1] = 0.0;  vertices[5][2] = 0.0;  /* closing */

    *vindex_out = vindex;
    *vertices_out = vertices;
}

/* ------------------------------------------------------------------ */

int main(void)
{
    int passed = 1;
    int result;

    fprintf(stderr, "=== inpoly_test ===\n");

    /* ---- Test 1: inpoly — center of unit square ---- */
    fprintf(stderr, "\n--- Test 1: inpoly — center of unit square ---\n");
    {
        double **poly = make_unit_square_poly();

        result = inpoly(poly, UNIT_SQ_NPOINTS, 0.5, 0.5);
        fprintf(stderr, "  inpoly(0.5, 0.5) = %d (expected 1)\n", result);

        if (result == 1) {
            fprintf(stderr, "  OK\n");
        } else {
            fprintf(stderr, "  FAIL\n");
            passed = 0;
        }

        free_2d_double(poly, UNIT_SQ_NPOINTS);
    }

    /* ---- Test 2: inpoly — degenerate polygon (npoints < 3) ---- */
    fprintf(stderr, "\n--- Test 2: inpoly — degenerate polygon ---\n");
    {
        double **poly = alloc_2d_double(2, 2);
        poly[1][1] = 0.0;  poly[1][2] = 0.0;
        poly[2][1] = 1.0;  poly[2][2] = 1.0;

        result = inpoly(poly, 2, 0.5, 0.5);
        fprintf(stderr, "  inpoly(npoints=2) = %d (expected 0)\n", result);

        if (result == 0) {
            fprintf(stderr, "  OK: early return for degenerate polygon\n");
        } else {
            fprintf(stderr, "  FAIL\n");
            passed = 0;
        }

        free_2d_double(poly, 2);
    }

    /* ---- Test 3: pointinpoly — center of unit square ---- */
    fprintf(stderr, "\n--- Test 3: pointinpoly — center of unit square ---\n");
    {
        int **vindex;
        double **vertices;
        make_unit_square_dda(&vindex, &vertices);

        result = pointinpoly(1, 0.5, 0.5, vindex, vertices);
        fprintf(stderr, "  pointinpoly(1, 0.5, 0.5) = %d (expected 1)\n", result);

        if (result == 1) {
            fprintf(stderr, "  OK\n");
        } else {
            fprintf(stderr, "  FAIL\n");
            passed = 0;
        }

        free_2d_int(vindex, 1);
        free_2d_double(vertices, UNIT_SQ_NVERTS);
    }

    fprintf(stderr, "\n=== inpoly_test: %s ===\n", passed ? "PASSED" : "FAILED");
    return passed ? 0 : 1;
}
