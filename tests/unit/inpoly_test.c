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
 *
 * KNOWN BUG — inpoly() initialization (npoints-1 off-by-one):
 *
 *   inpoly() initializes the ray-cast with poly[npoints-1] (the
 *   second-to-last vertex) instead of poly[npoints] (the last
 *   vertex).  This creates a spurious diagonal edge from vertex
 *   npoints-1 to vertex 1, replacing the real closing edge from
 *   vertex npoints to vertex 1.  Points below this diagonal are
 *   misclassified as outside.
 *
 *   Workaround: call with a closing vertex (first vertex repeated
 *   at index npoints+1) so that poly[npoints-1] is the last unique
 *   vertex.  This is how DDA's pointinpoly() works — the DDA vertex
 *   layout always includes a closing vertex and padding.
 *
 *   Affected tests: 5 (unit square), 10 (L-shape), 12 (diagonal
 *   boundary), 13 (negative coordinates).
 *
 * DISCREPANCY — edge/vertex point classification:
 *
 *   The two functions disagree on points exactly on polygon edges.
 *   inpoly() classifies edge points as outside (strict < in the
 *   crossing test); pointinpoly() classifies some edge points as
 *   inside due to the extra zero-length edges from DDA padding.
 *   Neither is "correct" — point-on-boundary is implementation-
 *   defined for ray-casting algorithms.
 *
 *   Affected tests: 7 (vertex), 8 (horizontal edge), 9 (vertical edge).
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

    /* ============================================================
     * Pathological inputs: break the algorithm.
     *
     * inpoly() has an initialization bug: it starts the ray-cast
     * from poly[npoints-1] (second-to-last vertex) instead of
     * poly[npoints] (last vertex).  When called with npoints=4
     * (no closing vertex), the edges processed are:
     *
     *   (1,1)→(0,0)   spurious diagonal (vertex 3 to vertex 1)
     *   (0,0)→(1,0)   bottom ✓
     *   (1,0)→(1,1)   right  ✓
     *   (1,1)→(0,1)   top    ✓
     *
     * The left edge (0,1)→(0,0) is missing, replaced by a diagonal.
     * Points below the y=x diagonal inside the square get an extra
     * crossing and are misclassified as outside.
     * ============================================================ */

    /* ---- Test 4: point clearly outside ---- */
    fprintf(stderr, "\n--- Test 4: outside point (2, 2) ---\n");
    {
        double **poly = make_unit_square_poly();

        result = inpoly(poly, UNIT_SQ_NPOINTS, 2.0, 2.0);
        fprintf(stderr, "  inpoly(2, 2) = %d (expected 0)\n", result);
        if (result != 0) { fprintf(stderr, "  FAIL\n"); passed = 0; }
        else              { fprintf(stderr, "  OK\n"); }

        int **vi; double **vt;
        make_unit_square_dda(&vi, &vt);
        result = pointinpoly(1, 2.0, 2.0, vi, vt);
        fprintf(stderr, "  pointinpoly(2, 2) = %d (expected 0)\n", result);
        if (result != 0) { fprintf(stderr, "  FAIL\n"); passed = 0; }
        else              { fprintf(stderr, "  OK\n"); }

        free_2d_double(poly, UNIT_SQ_NPOINTS);
        free_2d_int(vi, 1);
        free_2d_double(vt, UNIT_SQ_NVERTS);
    }

    /* ---- Test 5: inpoly npoints-1 initialization bug ---- */
    fprintf(stderr, "\n--- Test 5: inpoly initialization bug (0.5, 0.4) ---\n");
    {
        /*
         * (0.5, 0.4) is inside the unit square.  But it's below the
         * y=x diagonal.  The spurious diagonal edge causes an extra
         * crossing, making the total even → outside.
         *
         * pointinpoly gets this right because the DDA vertex layout
         * includes a closing vertex and padding, so all 4 real edges
         * are present.
         */
        double **poly = make_unit_square_poly();

        result = inpoly(poly, UNIT_SQ_NPOINTS, 0.5, 0.4);
        fprintf(stderr, "  inpoly(0.5, 0.4) = %d\n", result);
        if (result == 0) {
            fprintf(stderr, "  BUG CONFIRMED: inside point classified as outside\n");
            fprintf(stderr, "  Cause: poly[npoints-1] init creates spurious diagonal\n");
        } else {
            fprintf(stderr, "  Unexpected: point classified correctly\n");
        }

        int **vi; double **vt;
        make_unit_square_dda(&vi, &vt);
        result = pointinpoly(1, 0.5, 0.4, vi, vt);
        fprintf(stderr, "  pointinpoly(0.5, 0.4) = %d (expected 1)\n", result);
        if (result != 1) { fprintf(stderr, "  FAIL\n"); passed = 0; }
        else              { fprintf(stderr, "  OK: DDA layout handles this correctly\n"); }

        free_2d_double(poly, UNIT_SQ_NPOINTS);
        free_2d_int(vi, 1);
        free_2d_double(vt, UNIT_SQ_NVERTS);
    }

    /* ---- Test 6: inpoly bug with closing vertex fix ---- */
    fprintf(stderr, "\n--- Test 6: inpoly with closing vertex (npoints=5) ---\n");
    {
        /*
         * Adding a closing vertex (vertex 5 = vertex 1) makes
         * poly[npoints-1] = poly[4] = the actual last unique vertex.
         * This gives correct edges.  The closing vertex creates a
         * duplicate left edge, but it's vertical at x=0, so it
         * never triggers a crossing for interior points.
         */
        double **poly = alloc_2d_double(5, 2);
        poly[1][1] = 0.0;  poly[1][2] = 0.0;
        poly[2][1] = 1.0;  poly[2][2] = 0.0;
        poly[3][1] = 1.0;  poly[3][2] = 1.0;
        poly[4][1] = 0.0;  poly[4][2] = 1.0;
        poly[5][1] = 0.0;  poly[5][2] = 0.0;  /* closing vertex */

        result = inpoly(poly, 5, 0.5, 0.4);
        fprintf(stderr, "  inpoly(0.5, 0.4, npoints=5) = %d (expected 1)\n", result);
        if (result == 1) {
            fprintf(stderr, "  OK: closing vertex fixes the initialization bug\n");
        } else {
            fprintf(stderr, "  FAIL\n");
            passed = 0;
        }

        free_2d_double(poly, 5);
    }

    /* ---- Test 7: point on vertex ---- */
    fprintf(stderr, "\n--- Test 7: point on vertex (0, 0) ---\n");
    {
        double **poly = make_unit_square_poly();

        result = inpoly(poly, UNIT_SQ_NPOINTS, 0.0, 0.0);
        fprintf(stderr, "  inpoly(0, 0) = %d (on-vertex: implementation-defined)\n", result);

        int **vi; double **vt;
        make_unit_square_dda(&vi, &vt);
        int result2 = pointinpoly(1, 0.0, 0.0, vi, vt);
        fprintf(stderr, "  pointinpoly(0, 0) = %d\n", result2);

        if (result != result2)
            fprintf(stderr, "  NOTE: functions disagree on vertex point\n");
        else
            fprintf(stderr, "  Both agree: %s\n", result ? "inside" : "outside");

        free_2d_double(poly, UNIT_SQ_NPOINTS);
        free_2d_int(vi, 1);
        free_2d_double(vt, UNIT_SQ_NVERTS);
    }

    /* ---- Test 8: point on edge ---- */
    fprintf(stderr, "\n--- Test 8: point on bottom edge (0.5, 0) ---\n");
    {
        double **poly = make_unit_square_poly();

        result = inpoly(poly, UNIT_SQ_NPOINTS, 0.5, 0.0);
        fprintf(stderr, "  inpoly(0.5, 0) = %d (on-edge: implementation-defined)\n", result);

        int **vi; double **vt;
        make_unit_square_dda(&vi, &vt);
        int result2 = pointinpoly(1, 0.5, 0.0, vi, vt);
        fprintf(stderr, "  pointinpoly(0.5, 0) = %d\n", result2);

        if (result != result2)
            fprintf(stderr, "  NOTE: functions disagree on edge point\n");
        else
            fprintf(stderr, "  Both agree: %s\n", result ? "inside" : "outside");

        free_2d_double(poly, UNIT_SQ_NPOINTS);
        free_2d_int(vi, 1);
        free_2d_double(vt, UNIT_SQ_NVERTS);
    }

    /* ---- Test 9: point on right edge (vertical) ---- */
    fprintf(stderr, "\n--- Test 9: point on right edge (1.0, 0.5) ---\n");
    {
        double **poly = make_unit_square_poly();

        result = inpoly(poly, UNIT_SQ_NPOINTS, 1.0, 0.5);
        fprintf(stderr, "  inpoly(1.0, 0.5) = %d (on-edge: implementation-defined)\n", result);

        int **vi; double **vt;
        make_unit_square_dda(&vi, &vt);
        int result2 = pointinpoly(1, 1.0, 0.5, vi, vt);
        fprintf(stderr, "  pointinpoly(1.0, 0.5) = %d\n", result2);

        if (result != result2)
            fprintf(stderr, "  NOTE: functions disagree on edge point\n");
        else
            fprintf(stderr, "  Both agree: %s\n", result ? "inside" : "outside");

        free_2d_double(poly, UNIT_SQ_NPOINTS);
        free_2d_int(vi, 1);
        free_2d_double(vt, UNIT_SQ_NVERTS);
    }

    /* ---- Test 10: concave L-shape polygon ---- */
    fprintf(stderr, "\n--- Test 10: concave L-shape ---\n");
    {
        /*
         * L-shape (6 vertices, no closing vertex):
         *
         *   6=(0,2)---5=(1,2)
         *     |         |
         *   1=(0,0)   4=(1,1)---3=(2,1)
         *     |                   |
         *     +--------2=(2,0)---+
         *
         * Vertices CCW: (0,0) (2,0) (2,1) (1,1) (1,2) (0,2)
         *
         * npoints-1 bug: poly[5]=(1,2), not poly[6]=(0,2).
         * Spurious diagonal from (1,2) to (0,0) replaces the
         * left edge from (0,2) to (0,0).  Points in the bottom
         * arm below that diagonal are misclassified.
         */
        double **poly = alloc_2d_double(6, 2);
        poly[1][1] = 0.0;  poly[1][2] = 0.0;
        poly[2][1] = 2.0;  poly[2][2] = 0.0;
        poly[3][1] = 2.0;  poly[3][2] = 1.0;
        poly[4][1] = 1.0;  poly[4][2] = 1.0;
        poly[5][1] = 1.0;  poly[5][2] = 2.0;
        poly[6][1] = 0.0;  poly[6][2] = 2.0;

        /* (0.5, 0.5) — in the bottom arm, below the spurious diagonal.
         * Diagonal from (1,2) to (0,0) has y = 2x.  At x=0.5, y=1.0.
         * Point at y=0.5 is below → misclassified.
         */
        result = inpoly(poly, 6, 0.5, 0.5);
        fprintf(stderr, "  inpoly(0.5, 0.5) L-bottom = %d\n", result);
        if (result == 0)
            fprintf(stderr, "  BUG CONFIRMED: inside point in L-arm misclassified\n");
        else
            fprintf(stderr, "  OK\n");

        /* (1.5, 0.5) — in the bottom-right, should be inside.
         * At x=1.5, the diagonal y=2x gives y=3.0, which is outside
         * the polygon entirely.  The spurious diagonal doesn't span
         * this x-range, so this point is unaffected.
         */
        result = inpoly(poly, 6, 1.5, 0.5);
        fprintf(stderr, "  inpoly(1.5, 0.5) L-right = %d (expected 1)\n", result);
        if (result != 1) { fprintf(stderr, "  FAIL\n"); passed = 0; }
        else              { fprintf(stderr, "  OK\n"); }

        /* (1.5, 1.5) — in the concavity, should be outside */
        result = inpoly(poly, 6, 1.5, 1.5);
        fprintf(stderr, "  inpoly(1.5, 1.5) L-concavity = %d (expected 0)\n", result);
        if (result != 0) { fprintf(stderr, "  FAIL\n"); passed = 0; }
        else              { fprintf(stderr, "  OK\n"); }

        /* (0.5, 1.5) — in the top arm, should be inside */
        result = inpoly(poly, 6, 0.5, 1.5);
        fprintf(stderr, "  inpoly(0.5, 1.5) L-top = %d (expected 1)\n", result);
        if (result != 1) { fprintf(stderr, "  FAIL\n"); passed = 0; }
        else              { fprintf(stderr, "  OK\n"); }

        free_2d_double(poly, 6);
    }

    /* ---- Test 11: collinear vertices (degenerate polygon) ---- */
    fprintf(stderr, "\n--- Test 11: collinear vertices (line, not polygon) ---\n");
    {
        /* Three points on the x-axis: a line segment, zero area. */
        double **poly = alloc_2d_double(3, 2);
        poly[1][1] = 0.0;  poly[1][2] = 0.0;
        poly[2][1] = 1.0;  poly[2][2] = 0.0;
        poly[3][1] = 2.0;  poly[3][2] = 0.0;

        result = inpoly(poly, 3, 1.0, 0.0);
        fprintf(stderr, "  inpoly(1, 0) on collinear = %d (expected 0)\n", result);
        if (result == 0)
            fprintf(stderr, "  OK: zero-area polygon returns outside\n");
        else
            fprintf(stderr, "  NOTE: collinear polygon returns inside\n");

        result = inpoly(poly, 3, 1.0, 0.5);
        fprintf(stderr, "  inpoly(1, 0.5) above collinear = %d (expected 0)\n", result);
        if (result == 0)
            fprintf(stderr, "  OK\n");
        else {
            fprintf(stderr, "  FAIL: point above flat line classified as inside\n");
            passed = 0;
        }

        free_2d_double(poly, 3);
    }

    /* ---- Test 12: exactly on the diagonal (boundary of bug) ---- */
    fprintf(stderr, "\n--- Test 12: point on spurious diagonal (0.5, 0.5) ---\n");
    {
        /*
         * The spurious diagonal from (1,1) to (0,0) has equation y=x.
         * The point (0.5, 0.5) is exactly on it.  The crossing test
         * uses strict < so it doesn't count.  This means (0.5, 0.5)
         * returns 1 (the center test) but (0.5, 0.499) returns 0.
         */
        double **poly = make_unit_square_poly();

        result = inpoly(poly, UNIT_SQ_NPOINTS, 0.5, 0.499);
        fprintf(stderr, "  inpoly(0.5, 0.499) = %d\n", result);
        if (result == 0)
            fprintf(stderr, "  BUG CONFIRMED: just below diagonal → outside\n");
        else
            fprintf(stderr, "  Unexpected: classified as inside\n");

        result = inpoly(poly, UNIT_SQ_NPOINTS, 0.5, 0.501);
        fprintf(stderr, "  inpoly(0.5, 0.501) = %d (expected 1)\n", result);
        if (result == 1)
            fprintf(stderr, "  OK: just above diagonal → inside\n");
        else {
            fprintf(stderr, "  FAIL\n");
            passed = 0;
        }

        free_2d_double(poly, UNIT_SQ_NPOINTS);
    }

    /* ---- Test 13: negative coordinates ---- */
    fprintf(stderr, "\n--- Test 13: negative coordinates ---\n");
    {
        /*
         * Square centered at origin: (-1,-1) (1,-1) (1,1) (-1,1)
         * Spurious diagonal: poly[3]=(1,1) to poly[1]=(-1,-1), i.e. y=x.
         * (-0.5, -0.5) is ON the diagonal (y=x), so strict < doesn't
         * count it.  Use (-0.5, -0.6) which is below y=x.
         */
        double **poly = alloc_2d_double(4, 2);
        poly[1][1] = -1.0;  poly[1][2] = -1.0;
        poly[2][1] =  1.0;  poly[2][2] = -1.0;
        poly[3][1] =  1.0;  poly[3][2] =  1.0;
        poly[4][1] = -1.0;  poly[4][2] =  1.0;

        result = inpoly(poly, 4, 0.0, 0.0);
        fprintf(stderr, "  inpoly(0, 0) centered square = %d (expected 1)\n", result);
        if (result != 1) { fprintf(stderr, "  FAIL\n"); passed = 0; }
        else              { fprintf(stderr, "  OK\n"); }

        result = inpoly(poly, 4, -0.5, -0.6);
        fprintf(stderr, "  inpoly(-0.5, -0.6) = %d\n", result);
        if (result == 0)
            fprintf(stderr, "  BUG CONFIRMED: npoints-1 bug hits negative coords too\n");
        else
            fprintf(stderr, "  OK\n");

        free_2d_double(poly, 4);
    }

    fprintf(stderr, "\n=== inpoly_test: %s ===\n", passed ? "PASSED" : "FAILED");
    return passed ? 0 : 1;
}
