/*
 * wingraph.c: code for graphics modules of Windows 
 * version of DDA.
 */

#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "dda.h"
#include "wingraph.h"
#include "geometrydata.h"


/* GEOMSCALE is for printing */
#define GEOMSCALE 1
#define MAXVERTICES 35

char mess[80];
char scaletext[10];

extern FILEPOINTERS fp;


int 
setScale(HWND hw, HDC hdc, GRAPHICS * g, double scale_params[4]) 
{
   RECT winSize;
   long xscale, yscale, scale;
   double minX, minY;
   double w1, w2, w3, w4;

   extern long zoom;

  /* These are set in winmain, first as default, then 
   * handled by mouse movement.
   */
   extern int xoff, yoff;

   assert (hdc != NULL);

  /* FIXME: Change variable names to reflect function.
   */
   w1 = scale_params[0];  // min x
   w2 = scale_params[1];  // max x
   w3 = scale_params[2];  // min y
   w4 = scale_params[3];  // max y

   //xscale = (long) (25000/(w2-w1) + 0.5);
	  //yscale = (long) (25000/(w4-w3) + 0.5);

  /* FIXME: Explain the .5 or get rid of it. */
   //xscale = (long) (zoom/(w2-w1) + 0.5);
	  //yscale = (long) (zoom/(w4-w3) + 0.5);

   xscale = (long) (zoom/(w2-w1));
	  yscale = (long) (zoom/(w4-w3));

	  scale = (xscale <= yscale)? xscale : yscale;

	  minX = w1*scale;
	  minY = w3*scale;

	  //g->offset.x = (long)(2500 - minX);
	  //g->offset.y = (long)(2500 - minY);
	  //g->radius = 125;  // circle radius = 5% of screen width

	  g->offset.x = (long)(xoff - minX);
	  g->offset.y = (long)(yoff - minY);

	  //g->offset.x = (double)(xoff - minX);
	  //g->offset.y = (double)(yoff - minY);

 

  	SetMapMode(hdc, MM_ISOTROPIC);
  	GetClientRect(hw, &winSize);

  /* FIXME: Make the radius scale with the problem scale. */
	  //g->radius = 125;  // circle radius = 5% of screen width
  /* FIXME: This is *not* .5% of screen width. It is an ugly kludge 
   * that is not quite as ugly as selecting the arbitrary value
   * of 0.05*2500 of the original technique.
   */
   g->radius = 0.005 * zoom;


  /* FIXME: Get rid of these hardwired values. */
  /* The NULL would be a pointer to a point struct
   * containing previous window extents.  The values
   * are given in logical coordinates.
   */
  	SetWindowExtEx(hdc, 30000, 30000, NULL);

  /* Translate the window origin to 0,0.  This 
   * could be fixed to have logical units pointing 
   * positive y up from bottom instead of positive 
   * y down from top. 
   */
  	SetWindowOrgEx(hdc, 0, 0, NULL);

  /* Draw right side which is upside down. */
  	SetViewportExtEx(hdc, winSize.right, -winSize.bottom, NULL); 
  	SetViewportOrgEx(hdc, 0, winSize.bottom, NULL);
  /* Draw upside down which is right side up. */
  	//SetViewportExtEx(hdc, winSize.right, winSize.top, NULL); 
  	//SetViewportOrgEx(hdc, 0, winSize.top, NULL);

  	//SetViewportOrgEx(hdc, (int)w1, winSize.bottom, NULL);

  	return scale;

}  /* close setScale( ) */



int 
drawLines(HDC hdc, HPEN hpen[10], double scale, POINT offset, 
          Joint *j, int numlines, int color) 
{
   int i, ii;
   POINT p1, p2;
	
   SelectObject(hdc, hpen[0]);
   for (i = 0; i< numlines; i++) 
   {

      if(color && (i==0 || j[i].type != j[i-1].type)) 
      {
		  ii = j[i].type;
		  if(ii < 0 || ii > 8) 
           ii = 9;
        SelectObject(hdc, hpen[ii]);
      }  

	    p1.x = (int)(j[i].d1.x*scale + .5 + offset.x);
	    p1.y = (int)(j[i].d1.y*scale + .5 + offset.y);
	    p2.x = (int)(j[i].d2.x*scale + .5 + offset.x);
	    p2.y = (int)(j[i].d2.y*scale + .5 + offset.y);

	    MoveToEx(hdc, p1.x, p1.y, NULL);
	    LineTo(hdc, p2.x, p2.y);
   }  // end for() each joint
	
   FillPath(hdc);
	
   return (0);

}  // Close drawLines()



int 
drawJoints(HDC hdc, HPEN hpen[10], GRAPHICS *g, Geometrydata * gd, 
           double ** d, int orig) 
{
   int i, ii;
   POINT p1, p2;

   int j, numSides, firstvertex;
   int ** k0 = gd->vindex;
   double scale = g->scale;
   int numblocks = gd->nBlocks;
   POINT offset = g->offset;
   //HGDIOBJ horigpen;

   //horigpen = 
   SelectObject(hdc, hpen[0]);

   for (i = 0; i< numblocks; i++) 
   {  
      firstvertex = k0[i+1][1];
      numSides = k0[i+1][2] - firstvertex + 1;
      for (j = 0; j< numSides; j++) 
      { 
         if(!orig)
         {
            if( !j || d[firstvertex+j+1][0] != d[firstvertex+j][0] )
            {
			      ii = (int)(d[firstvertex+j+1][0]+0.1);
			      if(ii < 0 || ii > 8) 
                  ii = 9;
			      //horigpen = 
                  SelectObject(hdc, hpen[ii]);
            }
         }  
      
         p1.x = (long)(d[firstvertex+j][1]*scale + .5 + offset.x);
         p1.y = (long)(d[firstvertex+j][2]*scale + .5 + offset.y);
         p2.x = (long)(d[firstvertex+j+1][1]*scale + .5 + offset.x);
         p2.y = (long)(d[firstvertex+j+1][2]*scale + .5 + offset.y);
         MoveToEx(hdc, p1.x, p1.y, NULL);
         LineTo(hdc, p2.x, p2.y);

      //SelectObject(hdc,horigpen);
      //DeleteObject(hpen[ii]);

      }  // end for() each joint
   } /* end for each block*/

   FillPath(hdc);
	
   return (0);

}  // Close drawJoints()


int 
drawPoints(HDC hdc, GRAPHICS * g, Geometrydata * gd, double ** points) 
{
   int i;
   HPEN hpen;
   HBRUSH hbr;
   POINT pt[3];

  /* GDI example on p. 299 of R&N. */
   //HGDIOBJ horigpen, horigbrush;

   double scale = g->scale;
   POINT offset = g->offset;
   double radius = g->radius;
   int numpoints = gd->nFPoints+gd->nLPoints+gd->nMPoints;

   hpen = GetStockObject(BLACK_PEN);
   SelectObject(hdc, hpen);
   
  /* FIXME:  numpoints includes hole points, which we 
   * probably don't want to draw.  We (hopefully) got 
   * really lucky here when GHS decided to load hole
   * points into the end of the points array.  Otherwise,
   * this would require a check, or rewriting the input
   * file format.
   */
   //for(i=0; i< numpoints-gd->nHPoints; i++) 
   for(i=0; i< numpoints; i++) 
   {
     /* This is a bit kludgy ad non-obvious.
      * FIXME: with self-documenting code.
      */
      if(!i || points[i+1][0] != points[i][0]) 
      {
		   hbr = GetStockObject(WHITE_BRUSH);  // fixed and meas
			if(points[i+1][0] > 1) 
            hbr = GetStockObject(BLACK_BRUSH); // load and hole
      //horigbrush = 
         SelectObject(hdc, hbr);
			      
      }
             

	   // hbr = GetStockObject(WHITE_BRUSH);  // fixed and meas
	   // SelectObject(hdc, hbr);

      if((int)points[i+1][0] == 0 || (int)points[i+1][0] == 2)
      {
        /* fixed and load: triangles */
	      pt[0].x = (int)(points[i+1][1] * scale + .5 + offset.x);
	      pt[0].y = (int)(points[i+1][2] * scale + .5 + offset.y + 1.2*radius);
		   pt[1].x = (int)(points[i+1][1] * scale + .5 + offset.x - 1.2*0.866*radius);
		   pt[1].y = (int)(points[i+1][2] * scale + .5 + offset.y - 1.2*0.5*radius);
		   pt[2].x = (int)(points[i+1][1] * scale + .5 + offset.x + 1.2*0.866*radius);
		   pt[2].y = (int)(points[i+1][2] * scale + .5 + offset.y - 1.2*0.5*radius);
		   SetPolyFillMode (hdc, WINDING) ;
		   Polygon(hdc, pt, 3);
      }
     /* Draw measured points but not hole points.
      */
      else //if( (int)points[i+1][0] == 1)
      {
         // meas and hole: circles
         //pt[0].x = (int)(p[i].x*scale + .5 + offset.x);
         //pt[0].y = (int)(p[i].y*scale + .5 + offset.y);
         pt[0].x = (int)(points[i+1][1]*scale + .5 + offset.x);
         pt[0].y = (int)(points[i+1][2]*scale + .5 + offset.y);
         Ellipse(hdc, (int) (pt[0].x-radius), (int) (pt[0].y+radius), (int) (pt[0].x+radius), (int) (pt[0].y-radius));
      }
   } // end for each point

   if (gd->seispoints != NULL)
   { 
      DList * ptr;
      DDAPoint * ptmp;

      hbr = GetStockObject(LTGRAY_BRUSH); // load and hole
		SelectObject(hdc, hbr);

      M_dl_traverse(ptr, gd->seispoints)
      {
         //Extract the point struct from the DList.
         ptmp = ptr->val;
	      pt[0].x = (int)(ptmp->x * scale + .5 + offset.x);
	      pt[0].y = (int)(ptmp->y * scale + .5 + offset.y + 1.2*radius);
		   pt[1].x = (int)(ptmp->x * scale + .5 + offset.x - 1.2*0.866*radius);
		   pt[1].y = (int)(ptmp->y * scale + .5 + offset.y - 1.2*0.5*radius);
		   pt[2].x = (int)(ptmp->x * scale + .5 + offset.x + 1.2*0.866*radius);
		   pt[2].y = (int)(ptmp->y * scale + .5 + offset.y - 1.2*0.5*radius);
		   SetPolyFillMode (hdc, WINDING);
		   Polygon(hdc, pt, 3);
      }
   }

   //SelectObject(hdc, horigpen);
   //DeleteObject(hpen);
   return(0);

}  // Close drawpoints()


int 
drawBlocks(HDC hdc, HBRUSH hbr[6], GRAPHICS * g, Geometrydata * gd) {

   int i, j, k;
   int numSides;  // length;
   int firstvertex;
   //FILE * blockmat;
   int numblocks = gd->nBlocks;

   //double offsetx = g->offsetx;
   //double offsety = g->offsety;

   double scale = g->scale;
   POINT offset = g->offset;
   POINT p[MAXVERTICES];

   //FILE * ofp;
   //HGDIOBJ horigpen, horigbrush, horigbrush2;
   //char c[80];

   int ** k0 = gd->vindex;
   double ** d = gd->vertices;

   //ofp = fopen("blocks","w");
   //fprintf(ofp,"Scale: %f, offx: %d, offy: %d\n",scale, g->offset.x, g->offset.y);

   assert (hdc != NULL);

   for (i = 0; i< numblocks; i++) 
   {
  		//hBr[0] = light blue
      //hBr[1] = light green
      //hBr[2] = light red
      //hBr[3] = light brown
      //hBr[4] = light purple
      //hBr[5] = light gray
      //fprintf(blockmat,"Block %d mat number: %d\n", i, k0[i+1][0]); 

      if( (i == 0) || k0[i+1][0] != k0[i][0]) 
      {
         j = k0[i+1][0];
         if(j < 0 || j > 4) 
            j = 5;
         	//horigbrush2 = 
         SelectObject(hdc, hbr[j]);
	   }
      
      firstvertex = k0[i+1][1];
      numSides = k0[i+1][2] - firstvertex + 1;
      //p = (POINT *) malloc(sizeof(POINT) * (numSides+1));

      for (k = 0; k<numSides; k++) 
      {
        /* FIXME: Get rid of the .5 or explain why it is 
         * necessary.
         */
         //p[k].x = (long)(d[firstvertex+k][1] * scale + .5 + offset.x);
         //p[k].y = (long)(d[firstvertex+k][2] * scale + .5 + offset.y);

        /* WARNING!!!  Too many vertices on a block will induce a segfault here.
         * For now, the maximum number of vertices is hard-wired with the MAXVERTICES
         * preprocessor definition.
         */
         p[k].x = (long)(d[firstvertex+k][1] * scale + offset.x);
         p[k].y = (long)(d[firstvertex+k][2] * scale + offset.y);
      }

      SetPolyFillMode (hdc, WINDING) ;
      Polygon(hdc, p, numSides);

      //SelectObject(hdc,horigbrush2);
      //DeleteObject(hbr[j]);

   } // end for each block



     /* Draw a bounding box. */
/*
      p[0].x = (long)(gd->scale[0] * scale + offset.x - 525);
      p[0].y = (long)(gd->scale[2] * scale + offset.y - 525);
      p[1].x = (long)(gd->scale[1] * scale + offset.x + 525);
      p[1].y = (long)(gd->scale[2] * scale + offset.y - 525);
      p[2].x = (long)(gd->scale[1] * scale + offset.x + 525);
      p[2].y = (long)(gd->scale[3] * scale + offset.y + 525);
      p[3].x = (long)(gd->scale[0] * scale + offset.x - 525);
      p[3].y = (long)(gd->scale[3] * scale + offset.y + 525);

      SetPolyFillMode (hdc, WINDING) ;
      Polygon(hdc, p, 4);
*/



   return(0);
}  /* Close  drawBlocks() */


int 
drawBolts(HDC hdc, HPEN hpen[6], Geometrydata * geomdata, GRAPHICS * g, double ** b) 
{
   int i;
   POINT p1, p2;

  /* FIXME: Get rid of this extern */
   //extern Geometrydata * geomdata;
   int numbolts = geomdata->nBolts;

   double scale = g->scale;
   POINT offset = g->offset;


   for (i = 0; i< numbolts; i++) 
   {
     /* Code for changing colors given different 
      * bolt types can be implemented following 
      * similar code in the drawBlocks function.
      */
		SelectObject(hdc, hpen[2]);
		p1.x = (int)(b[i][1]*scale + .5 + offset.x);
		p1.y = (int)(b[i][2]*scale + .5 + offset.y);
		p2.x = (int)(b[i][3]*scale + .5 + offset.x);
		p2.y = (int)(b[i][4]*scale + .5 + offset.y);
		MoveToEx(hdc, p1.x, p1.y, NULL);
		LineTo(hdc, p2.x, p2.y);
   }  /* Close for loop. */
   return 1;
}  /* Close drawBolts()  */





int 
drawCentroids(HDC hdc, HPEN hpen[9], Geometrydata * geomdata, GRAPHICS * g, Datalog * dl)
{   
   int i, j;
   POINT p1, p2;
	  double scale = g->scale;
   POINT offset = g->offset;
   DPoint ** c = g->centroids;
   //int ts = g->timestep;
  //extern Geometrydata * geomdata;
 
   SelectObject(hdc, hpen[5]);
   for (i = 1; i< g->numtimesteps-1; i++) 
   {
 		  	for (j=1;j<=geomdata->nBlocks;j++)
      {
		       p1.x = (int)((c[i][j].x)*scale + .5 + offset.x);
		       p1.y = (int)((c[i][j].y)*scale + .5 + offset.y);
		       p2.x = (int)((c[i+1][j].x)*scale + .5 + offset.x);
		       p2.y = (int)((c[i+1][j].y)*scale + .5 + offset.y);
		       MoveToEx(hdc, p1.x, p1.y, NULL);
		       LineTo(hdc, p2.x, p2.y);
      }
   }  // end for() each centroid
   FillPath(hdc);
   return 1;
}  /* Close drawCentroid() */


int 
drawStresses(HDC hdc, HPEN hpen[9], Geometrydata * geomdata, GRAPHICS * g)
{   
   int j;
   POINT p1, p2;
	  double scale = g->scale;
   POINT offset = g->offset;
   PStress ** s = g->stresses;
   int ts = g->timestep;
 
   if(ts == 0)
     return 1;

   SelectObject(hdc, hpen[5]);
 		  	for (j=1;j<=geomdata->nBlocks;j++)
      {
        /* "Major" stresses... */
		       p1.x = (int)((s[0][j].majorx1)*scale + .5 + offset.x);
		       p1.y = (int)((s[0][j].majory1)*scale + .5 + offset.y);
		       p2.x = (int)((s[0][j].majorx2)*scale + .5 + offset.x);
		       p2.y = (int)((s[0][j].majory2)*scale + .5 + offset.y);
		       MoveToEx(hdc, p1.x, p1.y, NULL);
		       LineTo(hdc, p2.x, p2.y);
      		
        /* "Minor" stresses... */
         p1.x = (int)((s[0][j].minorx1)*scale + .5 + offset.x);
		       p1.y = (int)((s[0][j].minory1)*scale + .5 + offset.y);
		       p2.x = (int)((s[0][j].minorx2)*scale + .5 + offset.x);
		       p2.y = (int)((s[0][j].minory2)*scale + .5 + offset.y);
		       MoveToEx(hdc, p1.x, p1.y, NULL);
		       LineTo(hdc, p2.x, p2.y);
      }  // end for each block
   FillPath(hdc);
   return 1;
}  /* Close drawStresses() */


HPEN drawPen[10];
HPEN screenPen[10];
HPEN printPen[10];
HBRUSH hBr[6];

#define PENWIDTH 100

void 
initializePens (void) 
{

			screenPen[0] = CreatePen(PS_DOT, 0, RGB(0,0,0));		// black
			screenPen[1] = CreatePen(PS_SOLID, PENWIDTH, RGB(0,0,0));		// black
			screenPen[2] = CreatePen(PS_SOLID, PENWIDTH, RGB(255,0,0));	// red
			screenPen[3] = CreatePen(PS_SOLID, PENWIDTH, RGB(0,0,255));	// blue
			screenPen[4] = CreatePen(PS_SOLID, PENWIDTH, RGB(0,255,64));	// green
			screenPen[5] = CreatePen(PS_SOLID, PENWIDTH, RGB(255,0,255));	// purple
			screenPen[6] = CreatePen(PS_SOLID, PENWIDTH, RGB(255,255,0));	// yellow
			screenPen[7] = CreatePen(PS_SOLID, PENWIDTH, RGB(128,64,0));	// brown
			screenPen[8] = CreatePen(PS_SOLID, PENWIDTH, RGB(255,128,0));	// olive
			screenPen[9] = CreatePen(PS_SOLID, PENWIDTH, RGB(192,192,192));	// gray

			//drawPen[0] = CreatePen(PS_DOT, 0, RGB(0,0,0));		// black
			drawPen[0] = CreatePen(PS_SOLID, 0, RGB(255,255,255));		// white
			drawPen[1] = CreatePen(PS_SOLID, 0, RGB(0,0,0));		// black
			drawPen[2] = CreatePen(PS_SOLID, 0, RGB(255,0,0));	// red
			drawPen[3] = CreatePen(PS_SOLID, 0, RGB(0,0,255));	// blue
			drawPen[4] = CreatePen(PS_SOLID, 0, RGB(0,255,64));	// green
			drawPen[5] = CreatePen(PS_SOLID, 0, RGB(255,0,255));	// purple
			drawPen[6] = CreatePen(PS_SOLID, 0, RGB(255,255,0));	// yellow
			drawPen[7] = CreatePen(PS_SOLID, 0, RGB(128,64,0));	// brown
			drawPen[8] = CreatePen(PS_SOLID, 0, RGB(255,128,0));	// olive
			drawPen[9] = CreatePen(PS_SOLID, 0, RGB(192,192,192));	// gray

			printPen[0] = CreatePen(PS_DOT, 0, RGB(0,0,0));		// black
			printPen[1] = CreatePen(PS_SOLID, 6, RGB(0,0,0));		// black
			printPen[2] = CreatePen(PS_SOLID, 6, RGB(255,0,0));	// red
			printPen[3] = CreatePen(PS_SOLID, 6, RGB(0,0,255));	// blue
			printPen[4] = CreatePen(PS_SOLID, 6, RGB(0,255,64));	// green
			printPen[5] = CreatePen(PS_SOLID, 6, RGB(255,0,255));	// purple
			printPen[6] = CreatePen(PS_SOLID, 6, RGB(255,255,0));	// yellow
			printPen[7] = CreatePen(PS_SOLID, 6, RGB(128,64,0));	// brown
			printPen[8] = CreatePen(PS_SOLID, 6, RGB(255,128,0));	// olive
			printPen[9] = CreatePen(PS_SOLID, 6, RGB(192,192,192));	// gray
} /*  Close initialize pens  */




void InitializeBrush ( )
{
	HBRUSH screenBrush[6];

			screenBrush[0] = CreateSolidBrush(RGB(192, 225, 255));	// light blue
			screenBrush[1] = CreateSolidBrush(RGB(192, 255, 225));	// light green
			screenBrush[2] = CreateSolidBrush(RGB(255, 192, 225));	// light red
			screenBrush[3] = CreateSolidBrush(RGB(225, 192, 160));	// light brown
			screenBrush[4] = CreateSolidBrush(RGB(225, 192, 255));	// light purple
			screenBrush[5] = CreateSolidBrush(RGB(225, 225, 225));	// light gray

}


void 
initializeBrushes()
{
			     hBr[0] = CreateSolidBrush(RGB(192, 225, 255));	// light blue
			     hBr[1] = CreateSolidBrush(RGB(192, 255, 225));	// light green
			     hBr[2] = CreateSolidBrush(RGB(255, 192, 225));	// light red
			     hBr[3] = CreateSolidBrush(RGB(225, 192, 160));	// light brown
			     hBr[4] = CreateSolidBrush(RGB(225, 192, 255));	// light purple
			     hBr[5] = CreateSolidBrush(RGB(225, 225, 225));	// light gray
}


double 
setPrScale(HWND hw, HDC hdc, GRAPHICS * g, double scale_params[4]) 
{
   double xscale, yscale;
	  double scale = 1.;
	  double minX, minY, xcenter, ycenter;
	  double unit;   
	  int xPage, yPage;
	  int xPixels, yPixels;
	  int devxOffset,devyOffset;
   //extern 
   char scaletext [10];
	  double w1, w2, w3, w4;
   
   POINT  offset;
   double radius;

 w1 = scale_params[0];
 w2 = scale_params[1];
 w3 = scale_params[2];
 w4 = scale_params[3];


//	xPage = GetDeviceCaps(hdc, HORZRES);
//	yPage = GetDeviceCaps(hdc, VERTRES);
	xPixels = GetDeviceCaps(hdc, LOGPIXELSX);
	yPixels = GetDeviceCaps(hdc, LOGPIXELSY);

	xPage = GetDeviceCaps(hdc, PHYSICALWIDTH );
	yPage = GetDeviceCaps(hdc, PHYSICALHEIGHT );

	devxOffset = GetDeviceCaps(hdc, PHYSICALOFFSETX );
	devyOffset = GetDeviceCaps(hdc, PHYSICALOFFSETY );


	xscale =  (xPage-2*xPixels)/(w2-w1) ;
	yscale =  (yPage-2*yPixels)/(w4-w3) ;
	
	scale = (xscale <= yscale)? xscale : yscale;

	
	//*pRadius = xPixels/40;                    // circle radius = .025"
	radius = xPixels/40;  // circle radius = 5% of screen width

	SetMapMode(hdc, MM_ISOTROPIC);
	SetWindowExtEx(hdc, xPage, yPage, NULL);
	SetViewportExtEx(hdc, xPage, -yPage, NULL);
	SetViewportOrgEx(hdc, 0 + xPixels, yPage - yPixels, NULL);


#if GEOMBORDER
{
	  MoveToEx(hdc, 0, 0, NULL);
	  LineTo(hdc, xPage - 2*xPixels, 0);
	  LineTo(hdc, xPage - 2*xPixels, yPage - 2*yPixels);
	  LineTo(hdc, 0, yPage - 2*yPixels);
	  LineTo(hdc, 0, 0);
}
#endif

	
#if GEOMSCALE
	{
char inchValue[256];

if (!strcmp(scaletext,"ft")){
		unit = xPixels/(scale);
		if (unit < 5) unit = 5;
		if ((unit > 5) && (unit < 10 )) {unit = 10;}
		if (unit > 10)
			if (unit < 20 ) unit = 20;
		if (unit > 20)
			if (unit < 30 ) unit = 30;
		if (unit > 30)
			if (unit < 40 ) unit = 40;
		if (unit > 40)
			if (unit < 50 ) unit = 50;
		if (unit > 50)
			if (unit < 75 ) unit = 75;
		if (unit > 75)
			if (unit < 100) unit = 100; 
		if (unit > 100)
			if (unit < 200) unit = 200;
		if (unit > 200) 
			if (unit < 250) unit = 250;
			else
			unit = 100* (int) ((unit/100)+.5);
		scale = xPixels/(unit);
		sprintf(inchValue, "Scale: 1 inch = %d %s", (int) unit, scaletext);
	}
	else {
		unit = xPixels/(2.54*scale);
		if (unit < 5) unit = 5;
		if (unit > 5)  
			if (unit < 10 ) unit = 10;
		if (unit > 10)
			if (unit < 20 ) unit = 20;
		if (unit > 20)
			if (unit < 30 ) unit = 30;
		if (unit > 30)
			if (unit < 40 ) unit = 40;
		if (unit > 40)
			if (unit < 50 ) unit = 50;
		if (unit > 50)
			if (unit < 75 ) unit = 75;
		if (unit > 75)
			if (unit < 100) unit = 100; 
		if (unit > 100)
			if (unit < 200) unit = 200;
		if (unit > 200) 
			if (unit < 250) unit = 250;
			else
			unit = 100* (int) ((unit/100)+.5);

		scale = xPixels/(2.54*unit);
		sprintf(inchValue, "Scale: 1 cm = %d %s", (int) unit, scaletext);
	}

//  Center the plot on a page

	xcenter = 0.5*(xPage-2*xPixels -  scale*(w2-w1));
	ycenter = 0.5*(yPage-2*yPixels -  scale*(w4-w3));

//  Correct for non-zero location of the lowermost corner

	minX =  w1*scale;
	minY =  w3*scale;

/*  Compute the plotting offset in device coordinates by combining the two 
	offsets above*/

	offset.x = (int)( xcenter- minX);
	offset.y = (int)( ycenter- minY);
	
		
//  This was the scale bar
//	MoveToEx(hdc, xPage - 2*xPixels, yPage - 2*yPixels, NULL);
//	LineTo(hdc, xPage - 3*xPixels, yPage - 2*yPixels);

	TextOut(hdc, (int) (xPage - 4*xPixels), 
		(int) (yPage - 2*yPixels - radius), 
		inchValue, strlen(inchValue));
	}
#endif

		 g->offset.x = offset.x; //(long)(2500 - minX);
	  g->offset.y = offset.y; //(long)(2500 - minY);
  /* FIXME: radius is broken.  Points come out huge. */
	  g->radius = radius;
   g->scale = scale;

	return scale;

}  /* close setPrScale() */