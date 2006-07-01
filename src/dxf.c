
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "dxf.h"


void
dxf_read_file (FILE * fp1, char *geofilename)
{

  double *jx1, *jy1, *jx2, *jy2;
  char str[5000][50];

  // Change to using ofp
  FILE *fp2;

  // output file pointer
  //FILE * ofp;

  int count1 = 0, count2 = 0, i, j;
  long *type, pn;
  int cabcou = 0, dec, ii, pltype, ctype, n1;
  int fixn = 0, measn = 0, holen = 0, loadn = 0;
  double r0, se, u, v, x1, x2, x3, y1, y2, y3, x0, y0, deg;
  double fx[25], fy[25], lx[25], ly[25];
  double mx[25], my[25], hx[25], hy[25];
  int nline = 0, npoly = 0, narc = 0, ncir = 0, ntext = 0;
  //OPENFILENAME ofn;
  //char temp[200];


  while (!feof (fp1)) {

    count1 += 1;
    fscanf (fp1, "%s", str[count1]);

    if (strcmp (str[count1], "LINE") == 0)
      nline += 1;

    if (strcmp (str[count1], "LWPOLYLINE") == 0)
      npoly += 1;

    if (strcmp (str[count1], "ARC") == 0)
      narc += 1;

    if (strcmp (str[count1], "CIRCLE") == 0)
      ncir += 1;
  }

  // Array Allocation
  n1 = nline + 40 * npoly + 40 * narc + 40 * ncir + 1;
  jx1 = (double *) calloc (n1, sizeof (double));
  jy1 = (double *) calloc (n1, sizeof (double));
  jx2 = (double *) calloc (n1, sizeof (double));
  jy2 = (double *) calloc (n1, sizeof (double));
  type = (long *) calloc (n1, sizeof (long));

  for (i = 1; i <= count1; i++) {

    // The dxf file will be read from this line and lines, polylines
    // and arcs will be recognized from dxf file

    if (strcmp (str[i], "LINE") == 0) {

      count2 += 1;
      if (strcmp (str[i + 9], "62") == 0) {

        type[count2] = atoi (str[i + 10]) + 1;
        dec = 2;
      } else {
        type[count2] = 1;
        dec = 0;
      }

      jx1[count2] = atof (str[i + 12 + dec]);
      jy1[count2] = atof (str[i + 14 + dec]);
      jx2[count2] = atof (str[i + 18 + dec]);
      jy2[count2] = atof (str[i + 20 + dec]);

    }

    if (strcmp (str[i], "LWPOLYLINE") == 0) {

      if (strcmp (str[i + 9], "62") == 0) {

        pltype = atoi (str[i + 10]) + 1;
        dec = 2;
      } else {

        pltype = 1;
        dec = 0;
      }
      pn = atoi (str[i + 12 + dec]);
      for (j = 0; j < pn - 1; j++) {
        ii = 4 * j;
        count2 += 1;
        jx1[count2] = atof (str[i + 18 + ii + dec]);
        jy1[count2] = atof (str[i + 18 + ii + 2 + dec]);
        jx2[count2] = atof (str[i + 18 + ii + 4 + dec]);
        jy2[count2] = atof (str[i + 18 + ii + 6 + dec]);
        type[count2] = pltype;
      }
      count2 += 1;
      if (strcmp (str[i + 9], "62") == 0) {
        type[count2] = atoi (str[i + 10]) + 1;
        dec = 2;
      } else {
        type[count2] = 1;
        dec = 0;
      }
      ii = 4 * 3;
      jx1[count2] = atof (str[i + 18 + ii + dec]);
      jy1[count2] = atof (str[i + 18 + ii + 2 + dec]);
      jx2[count2] = atof (str[i + 18 + dec]);
      jy2[count2] = atof (str[i + 20 + dec]);
      type[count2] = pltype;
    }
    // From this line first, center and second points
    // of an arc will be extracted from dxf
    // and the lines of arc are created with a
    // simple mathematical source 

    if (strcmp (str[i], "ARC") == 0) {
      if (strcmp (str[i + 9], "62") == 0) {
        ctype = atoi (str[i + 10]) + 1;
        dec = 2;
      } else {
        ctype = 1;
        dec = 0;
      }
      se = 30;
      deg = atof (str[i + 24 + dec]);
      r0 = ((3.1415926535) * deg) / (se * 180);
      x1 = atof (str[i + 12 + dec]) + atof (str[i + 18 + dec]);
      y1 = atof (str[i + 14 + dec]);
      x2 = atof (str[i + 12 + dec]);
      y2 = atof (str[i + 14 + dec]);
      x0 = x2;
      y0 = y2;
      x3 = atof (str[i + 12 + dec]) - atof (str[i + 18 + dec]);
      y3 = atof (str[i + 14 + dec]);

      for (j = 1; j < se; j++) {
        u = ((x1 - x0) * (cos (r0) - 1)) - ((y1 - y0) * sin (r0));
        v = ((x1 - x0) * sin (r0)) + ((y1 - y0) * (cos (r0) - 1));

        x2 = x1 + u;
        y2 = y1 + v;
        count2 += 1;
        jx1[count2] = x1;
        jy1[count2] = y1;
        jx2[count2] = x2;
        jy2[count2] = y2;
        type[count2] = ctype;
        x1 = x2;
        y1 = y2;

      }

      x2 = x3;
      y2 = y3;
      count2 += 1;
      jx1[count2] = x1;
      jy1[count2] = y1;
      jx2[count2] = x2;
      jy2[count2] = y2;
      type[count2] = ctype;
    }
    // From this line first, center and second
    // points of an circle will be extracted from dxf
    // and the lines of arc are created with a
    // simple mathematical source 
    if (strcmp (str[i], "CIRCLE") == 0) {
      if (strcmp (str[i + 9], "62") == 0) {
        ctype = atoi (str[i + 10]) + 1;
        dec = 2;
      } else {
        ctype = 1;
        dec = 0;
      }
      se = 30;
      deg = 360;
      r0 = ((3.1415926535) * deg) / (se * 180);
      x1 = atof (str[i + 12 + dec]) + atof (str[i + 18 + dec]);
      y1 = atof (str[i + 14 + dec]);
      x2 = atof (str[i + 12 + dec]);
      y2 = atof (str[i + 14 + dec]);
      x0 = x2;
      y0 = y2;
      x3 = x1;
      y3 = y1;

      for (j = 1; j < se; j++) {
        u = ((x1 - x0) * (cos (r0) - 1)) - ((y1 - y0) * sin (r0));
        v = ((x1 - x0) * sin (r0)) + ((y1 - y0) * (cos (r0) - 1));

        x2 = x1 + u;
        y2 = y1 + v;
        count2 += 1;
        jx1[count2] = x1;
        jy1[count2] = y1;
        jx2[count2] = x2;
        jy2[count2] = y2;
        type[count2] = ctype;
        x1 = x2;
        y1 = y2;

      }

      x2 = x3;
      y2 = y3;
      count2 += 1;
      jx1[count2] = x1;
      jy1[count2] = y1;
      jx2[count2] = x2;
      jy2[count2] = y2;
      type[count2] = ctype;
    }
    if (strcmp (str[i], "MTEXT") == 0) {
      if (strcmp (str[i + 9], "62") == 0) {
        dec = 2;
      } else {
        dec = 0;
      }
      if (strcmp (str[i + 26 + dec], "F") == 0) {
        fixn += 1;
        fx[fixn] = atof (str[i + 12 + dec]);
        fy[fixn] = atof (str[i + 14 + dec]);
      }
      if (strcmp (str[i + 26 + dec], "L") == 0) {
        loadn += 1;
        lx[loadn] = atof (str[i + 12 + dec]);
        ly[loadn] = atof (str[i + 14 + dec]);
      }
      if (strcmp (str[i + 26 + dec], "M") == 0) {
        measn += 1;
        mx[measn] = atof (str[i + 12 + dec]);
        my[measn] = atof (str[i + 14 + dec]);
      }
      if (strcmp (str[i + 26 + dec], "H") == 0) {
        holen += 1;
        hx[holen] = atof (str[i + 12 + dec]);
        hy[holen] = atof (str[i + 14 + dec]);
      }
    }
  }
  fclose (fp1);


  // From this line the data that extracted from dxf file will be sorted 
  // and will be wrote into geo file.

  fp2 = fopen (geofilename, "w");

  fprintf (fp2, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
  fprintf (fp2, "<!DOCTYPE DDA SYSTEM \"geometry.dtd\">\n");
  fprintf (fp2,
           "<Berkeley:DDA xmlns:Berkeley=\"http://www.tsoft.com/~bdoolin/dda\">\n");
  fprintf (fp2, "<!-- Bogus comment to keep ddaml tree-stripping\n");
  fprintf (fp2, "from seg faulting on bad child node. -->\n\n");
  fprintf (fp2, "<Geometry>\n");
  fprintf (fp2, "   <Edgenodedist distance=\"%lf\"/>\n", 0.001);
  fprintf (fp2, "   <Jointlist>\n");

  for (i = 1; i <= count2; i++) {
    fprintf (fp2, "      <Joint type=\"%d\">", type[i]);
    fprintf (fp2, "  %lf   %lf   %lf   %lf", jx1[i], jy1[i], jx2[i], jy2[i]);
    fprintf (fp2, "  </Joint>\n");
  }

  fprintf (fp2, "</Jointlist>\n");

  fprintf (fp2, "<Fixedpointlist>\n");

  for (i = 1; i <= fixn; i++) {
    fprintf (fp2, "<Line> %lf   %lf   %lf   %lf </Line>\n",
             fx[i], fy[i], fx[i], fy[i]);
  }

  fprintf (fp2, "</Fixedpointlist>\n");

  fprintf (fp2, "<Loadpointlist>\n");

  for (i = 1; i <= loadn; i++) {
    fprintf (fp2, "<Point> %lf   %lf  </Point>\n", lx[i], ly[i]);
  }

  fprintf (fp2, "</Loadpointlist>\n");

  fprintf (fp2, "<Measuredpointlist>\n");

  for (i = 1; i <= measn; i++) {
    fprintf (fp2, "<Point> %lf   %lf  </Point>\n", mx[i], my[i]);
  }

  fprintf (fp2, "</Measuredpointlist>\n");

  fprintf (fp2, "<Holepointlist>\n");

  for (i = 1; i <= holen; i++) {
    fprintf (fp2, "<Point> %lf   %lf  </Point>\n", hx[i], hy[i]);
  }

  fprintf (fp2, "</Holepointlist>\n");

  fprintf (fp2, "</Geometry>\n");
  fprintf (fp2, "</Berkeley:DDA>\n");

  fclose (fp2);



  free (jx1);
  free (jy1);
  free (jx2);
  free (jy2);
  free (type);

  return;

}



#ifdef STANDALONE
/** 
 * Standalone version of the dxf reader will translate from 
 * dxf to ddaml from the commandline.
 *
 * Compile in cygwin using:
 * gcc -o dxf  dxf.c -DSTANDALONE -I../include
 * 
 * Run it like this:
 * ./dxf < infile.dxf 
 */
int
main (int argc, char **argv)
{

  dxf_read_file (stdin, "foobar.geo");

  return 0;
}
#endif
