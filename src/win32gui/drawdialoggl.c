/* File contains functions to handle the creation and editing
 * of geometry files for DDA.  
 */

#include "drawdialog.h"
//#include <gl/gl.h>
#if OPENGL
#include <gl/glut.h>
#endif  

/* These 7 variables used to extern from main.  Some can probably
 * go local, others will be "global" in the sense of a class variable,
 * which will eventually go into a header file.
 */
static int gridSp=10;
static int newgridSp;
/* Lets get a whole bunch for now.  Array is the 
 * wrong data structure, linked list is better, but
 * that can be done later.
 */
int estNumPts = 200;
int estNumJts = 200;
/* Bounding box.  Was set in winmain.  Needs to be set 
 * here.  See note on DD_GRON  
 */
static int llx=0,lly=0,urx=100,ury=100;

/* FIXME: It would be much nicer to have the dialog box
 * start with a grid rather than without, but doing so
 * causes a lockup.
 */
static int grid=FALSE;
//static int grid=TRUE;

static double ScaleX, ScaleY;
static int ScaledX,ScaledY;

static enum TOOLTYPE  tool;

//static int tool=0;
static int type=1, inside = 0;
//static int grid=0;
static int  radius, maxSize;	

static HPEN hCurrentPen;
static HBRUSH hCurrentBr;
/* Global for handleInit.  These will 
 * get rolled into a struct at some point.
 */
static Joint *jp;
static DPoint *pp;
static int nLines=0, nPoints=0, nfp=0, nmp = 0, nlp = 0, nhp = 0;
static POINT  ptBegin, ptNew, ptOld;
static POINT tri[3];
static	char temp[80];
/* Global for handleInit */
static HDC hdc; 
//static HGLRC hRC1;


	//extern HPEN hPen[10];
 extern HPEN drawPen[10];
	extern FILEPATHS filepath;
	extern GRAPHICS * g;


BOOL CALLBACK DrawDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) 
{

  	switch (iMessage)
  	{
    		case WM_INITDIALOG:
	      		handleInit(hDlg);
      break;

	  	  case WM_COMMAND:
         return(handleWMCommand(hDlg, lParam, wParam));
 	  		break;

    		case WM_LBUTTONDOWN:
         handleLButtonDown(hDlg, lParam);
 		  	break;

	    	case WM_MOUSEMOVE:
         handleMouseMove(hDlg, lParam, wParam);
   			break;

   		 case WM_LBUTTONUP:
         handleLButtonUp(hDlg, lParam);
	   		break;

     /* As usual, add a destroy handler if this gets long */
      case WM_DESTROY:
        /* Put a message box here to test this out */
         //MessageBox(hDlg, "WM Destroy test", "WM_DESTROY", MB_OK); 
        /* Then free up all allocated memory. */
         freePointList();
         freeJointList();
      break;

     /* Probably should free some memory, etc. before 
      * shutting down.
      */
    		default:
	   		return FALSE;
   }
	  return TRUE;

  	ReleaseDC(hDlg, hdc);
  	ReleaseDC(GetDlgItem(hDlg,IDC_DRAWSPACE), hdc);

} /* close DrawDlgProc() */


static int
handleWMCommand(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   switch (LOWORD(wParam))
			{
		    case DD_J1: type = 1; tool=joint; hCurrentPen = drawPen[1]; break;
  			 case DD_J2: type = 2; tool=joint; hCurrentPen = drawPen[2]; break;
	     case DD_J3: type = 3; tool=joint; hCurrentPen = drawPen[3]; break;
     	case DD_J4: type = 4; tool=joint; hCurrentPen = drawPen[4]; break;
	     case DD_J5: type = 5; tool=joint; hCurrentPen = drawPen[5]; break;
	     case DD_J6: type = 6; tool=joint; hCurrentPen = drawPen[6]; break;
	     case DD_J7: type = 7; tool=joint; hCurrentPen = drawPen[7]; break;
	     case DD_J8: type = 8; tool=joint; hCurrentPen = drawPen[8]; break;
  				case DD_FP: type = 0; tool=fixedpoint; hCurrentPen = drawPen[1]; break;
     	case DD_MP: type = 1; tool=measpoint; hCurrentPen = drawPen[1]; break;
	   		case DD_LP: type = 2; tool=loadpoint; hCurrentPen = drawPen[1]; break;
  				case DD_HP: type = 3; tool=holepoint; hCurrentPen = drawPen[1]; break;
				
	    	case DD_BOLT1:
         handleRockbolts(hDlg);
      break;

	   		case DD_GRON:
         handleGridOn(hdc,jp,nLines,pp,nPoints);
	   		break;

	   		case DD_GROFF:
	       	handleGridOff(hdc,jp,nLines,pp,nPoints);
	  			break;

	   		case DD_REMJ:
         handleRemoveJoints(hDlg,hdc,jp);
 					break;

  				case DD_REMP:
         handleRemovePoints(hDlg,hdc,pp);
 					break;

     /* Handles changing grid spacing? 
      * TODO: Collapse this into a single callable function. 
      */
	 				case IDC_GRID:
	   	   	if (grid)
	   		   { 
		         	handleGridOff(hdc,jp,nLines,pp,nPoints);
	         		grid=TRUE;
	      		}			
		      	gridSp=changeGridSpacing(hDlg,lParam,wParam);

			      if (grid)
		      	{ 
		         	handleGridOn(hdc,jp,nLines,pp,nPoints);			
	      		}
 					break;

	 				case IDCANCEL:
	      		handleCancel(hDlg);
				  return FALSE;

				  case IDOK:
	       	return(handleSave(hDlg));

   }  /* end switch wParam */

}  /* close handleWMCommand() */


static void
handleLButtonDown(HWND hDlg, LPARAM lParam)
{
   SetCapture(hDlg);
  /* Get the (x,y) of the pointer when the left 
   * mouse button is pressed.
   */
   ptBegin.x = LOWORD(lParam); 
   ptBegin.y = HIWORD(lParam);

   DPtoLP(hdc, &ptBegin, 1);
   //ScreenToClient(hDlg,&ptBegin);
		 if(ptBegin.x <= maxSize && 
      ptBegin.x >= 0       && 
      ptBegin.y <= maxSize && 
      ptBegin.y >= 0        ) 
   {
			   ptOld = ptBegin;
			   inside = TRUE;
		 } 
   else 
   {
			   inside = FALSE;
   }

}  /* close handleMouseDown() */


static void
handleLButtonUp(HWND hDlg, LPARAM lParam)
{

#if OPENGL
  	double p1x,p1y,p2x,p2y;
	  float red, green, blue;
#endif

  /* These two lines are the only real platform dependent code in here.
   * Once the mouse position is grabbed, the math is similar.
   */
   ptNew.x = LOWORD(lParam); ptNew.y = HIWORD(lParam);
			DPtoLP(hdc, &ptNew, 1);

  /* We insist on actually being inside the drawing area...
   */
			if(ptNew.x <= maxSize && 
      ptNew.x >= 0       && 
      ptNew.y <= maxSize && 
      ptNew.y >= 0       && 
      inside) 
   {
			   SetROP2(hdc, R2_COPYPEN);
				  SelectObject(hdc, hCurrentPen);

     /* TODO: Turn this into a swicth on tool. */
			  	if(tool==joint) 
         addJoint(hDlg);
      else if (tool==fixedpoint || tool==loadpoint) 
         addTriangle(hDlg);
      else if (tool==measpoint || tool==holepoint) 
         addCircle(hDlg);
  	} 
   else /* outside drawing area */
   { 
	     SetROP2(hdc, R2_NOT);
    		if(!tool && inside) 
      {
	    				MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
	     			LineTo(hdc, ptOld.x, ptOld.y);
	   		}
	 	}
	 	ReleaseCapture();
//			ReleaseDC(hDlg, hdc);

}  /* close handleMouseUp() */


static void
addJoint(HWND hDlg)
{
   Joint * newjoint;
  /* Ok, here is the new stuff, just for the joints for now...
   * (Welcome to the wild world of data structures.)
   */
   if( (ptBegin.x != ptNew.x) ||  
       (ptBegin.y != ptNew.y)  )
   {
     /* Grab a new joint.. */
      newjoint = (Joint *)calloc(1,sizeof(Joint));
     /* Set the endpoints */
			   newjoint->d1.x = ptBegin.x;
		 			newjoint->d1.y = ptBegin.y;
						newjoint->d2.x = ptNew.x;
	  			newjoint->d2.y = ptNew.y;
		 			newjoint->type = type;
      //nLines++;  //May not need this.
     /* Now add this turkmeister to the tail of the 
      * ddadlist type jointlist...
      */
      dl_insert_b(jointlist, (void *)newjoint);
   }
      

/* Old stuff.  Keep this as duplicate, then 
 * #define away, then delete.
 */
   if(nLines < estNumJts && 
      (ptBegin.x != ptNew.x || 
       ptBegin.y != ptNew.y)) 
   {
      //MessageBox(hDlg, "Added one Joint", "Add", MB_OK);
			   jp[nLines].d1.x = ptBegin.x;
		 			jp[nLines].d1.y = ptBegin.y;
						jp[nLines].d2.x = ptNew.x;
	  			jp[nLines].d2.y = ptNew.y;
		 			jp[nLines].type = type;
		 			nLines++;
			} 
  /* We certainly will NOT need this after the array based
   * implementation is replaced.
   */
   else if(nLines >= estNumJts) 
   {
	     MessageBox(hDlg, "Estimated # of Joints Exceeded", "Too many joints", MB_OK);
	   		SetROP2(hdc, R2_NOT);
			}

  /* Now draw to screen */
 		MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
 		LineTo(hdc, ptNew.x, ptNew.y);

#if OPENGL
		 glPushMatrix();
	  glBegin(GL_LINES);
	  red=g->R[type];green=g->G[type];blue=g->B[type];
	  glColor3f( red, green, blue );
   p1x = (double)ptBegin.x/(double)maxSize;
	 	p1y =  (double)ptBegin.y/(double)maxSize;
			p2x =  (double)ptNew.x/(double)maxSize;
			p2y =  (double)ptNew.y/(double)maxSize;
	 	glVertex2d( p1x, p1y);
	 	glVertex2d( p2x, p2y);
	 	glEnd();
  	glPopMatrix();
	 	SwapBuffers(hdc);
#endif 

}  /* close addJoint() */


static void 
addTriangle(HWND hDlg)
{

   if(nPoints < estNumPts) 
   {
      pp[nPoints].x = ptNew.x;
		    pp[nPoints].y = ptNew.y;
	     pp[nPoints].type = type;
		    nPoints++;
				  if(tool==3) 
         nlp++;
					 else 
         nfp++;
					 hCurrentBr = GetStockObject(WHITE_BRUSH);
					 if(tool==3) 
         hCurrentBr = GetStockObject(BLACK_BRUSH);
				  SelectObject(hdc, hCurrentBr);
		    tri[0].x = (int) (ptNew.x);
		    tri[0].y = (int) (ptNew.y + 1.2*radius);
		    tri[1].x = (int) (ptNew.x - 1.2*0.866*radius+.5);
		    tri[1].y = (int) (ptNew.y - 1.2*0.5*radius);
      tri[2].x = (int) (ptNew.x + 1.2*0.866*radius);
		    tri[2].y = (int) (ptNew.y - 1.2*0.5*radius);
				  SetPolyFillMode (hdc, WINDING) ;
			   Polygon(hdc, tri, 3);
			 } 
    else 
    {
			    MessageBox(hDlg, "Estimated # of Points Exceeded", "Too many points", MB_OK);
		 	}

}  /* close addTriangle() */


static void
addCircle(HWND hDlg)
{

   if(nPoints < estNumPts) 
   {
			   pp[nPoints].x = ptNew.x;
			   pp[nPoints].y = ptNew.y;
      pp[nPoints].type = type;
			   nPoints++;
			   if(tool == holepoint) 
      {         
         hCurrentBr = GetStockObject(BLACK_BRUSH);
         nhp++;
      }
			  	else /* tool == measured points */
      {			   
         hCurrentBr = GetStockObject(WHITE_BRUSH);
         nmp++;
      }
			   SelectObject(hdc, hCurrentBr);
			   Ellipse(hdc, ptNew.x-radius, ptNew.y+radius, ptNew.x+radius, ptNew.y-radius);
	   } 
    else 
    {
	     	MessageBox(hDlg, "Estimated # of Points Exceeded", "Too many points", MB_OK);
	   }

}  /* close addCircle() */


static void 
handleMouseMove(HWND hDlg, LPARAM lParam, WPARAM wParam)
{

#if OPENGL
  	double p1x,p1y,p2x,p2y;
	  float red, green, blue;
#endif

			ptNew.x = LOWORD(lParam); ptNew.y = HIWORD(lParam);
			
			DPtoLP(hdc, &ptNew, 1);
			if(ptNew.x <= maxSize && ptNew.x >= 0 && ptNew.y <= maxSize && ptNew.y >= 0 && inside) {

// rescale coordinates
				ScaledX=(int)(ptNew.x*(ScaleX/maxSize));
				ScaledY=(int)(ptNew.y*(ScaleY/maxSize));

//				SetDlgItemInt(hDlg, DD_X, ptNew.x,TRUE);
//				SetDlgItemInt(hDlg, DD_Y, ptNew.y,TRUE);

				SetDlgItemInt(hDlg, DD_X, ScaledX,TRUE);
				SetDlgItemInt(hDlg, DD_Y, ScaledY,TRUE);


				SelectObject(hdc, hCurrentPen);
				if(wParam & MK_LBUTTON)
				{
					SetROP2(hdc, R2_NOT);
					if(!tool) {
						MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
						LineTo(hdc, ptOld.x, ptOld.y);
						MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
						LineTo(hdc, ptNew.x, ptNew.y);

#if OPENGL
						glPushMatrix();
			   glBegin(GL_LINES);
			   red=g->R[type];green=g->G[type];blue=g->B[type];
			   glColor3f( red, green, blue );
  			 p1x = (double)ptBegin.x/(double)maxSize;
	   		p1y =  (double)ptBegin.y/(double)maxSize;
	   		p2x =  (double)ptOld.x/(double)maxSize;
	   		p2y =  (double)ptOld.y/(double)maxSize;
		   	glVertex2d( p1x, p1y);
		   	glVertex2d( p2x, p2y);
			   red=g->R[9];green=g->G[9];blue=g->B[9];
			   glColor3f( red, green, blue );
	   		p2x =  (double)ptNew.x/(double)maxSize;
	   		p2y =  (double)ptNew.y/(double)maxSize;
		   	glVertex2d( p1x, p1y);
		   	glVertex2d( p2x, p2y);
	    	glEnd();
	   		glPopMatrix();
   			SwapBuffers(hdc);
#endif

   			}
					ptOld = ptNew;
				}
			} else { // outside drawing area
				// set dialog item text to blank
				temp[0] = '\0';
				SetDlgItemText(hDlg, DD_X, temp);
				SetDlgItemText(hDlg, DD_Y, temp);
				ptNew = ptOld;
			}
			// ReleaseDC(hDlg, hdc);

}  /* close handleMouseMove() */


/* Also sets radius variable used to draw points.
 */
static void
handleInit(HWND hDlg)
{
   RECT winSize;
	 	RECT drawSize;
		 int orig_x,orig_y,ext_x,ext_y;
	 	//char mess [240]; /*used for debugging message text*/
   //hdc = GetDC(hDlg);


   jointlist = make_dl();
   pointlist = make_dl();


			GetWindowRect(hDlg, &winSize);
			GetWindowRect(GetDlgItem(hDlg,IDC_DRAWSPACE), &drawSize);
			hdc = GetDC(GetDlgItem(hDlg,IDC_DRAWSPACE)); /*get device context of the draw area*/

			SetClassLong(GetDlgItem(hDlg,IDC_DRAWSPACE), GCL_HCURSOR, (long)LoadCursor(NULL,"IDC_CROSS "));

			hCurrentPen = drawPen[1];  // black
			
			SetMapMode(hdc, MM_ISOTROPIC);
						
  /*
			sprintf(mess, "Dialog top is %d bottom is %d\nDraw top is %d bottom is %d.\n\nClick to continue.", 
									winSize.top,winSize.bottom,drawSize.top,drawSize.bottom);
			MessageBox( hDlg, mess, "Geometry: Window Size", MB_ICONINFORMATION );
		 */
	
			orig_x= drawSize.left-winSize.left;
			orig_y= drawSize.top-winSize.top;
			ext_x=drawSize.right-drawSize.left;
			ext_y=drawSize.bottom-drawSize.top;

			
			SetWindowOrgEx(hdc,orig_x,orig_y, NULL);
			SetWindowExtEx(hdc,ext_x,ext_y,NULL);

			// want square area to draw in

			maxSize = ext_x-orig_x; /*> ury-lly ? urx : ury;*/
			if (ury-lly>maxSize) 
      maxSize=ext_y-orig_y; 


			SetViewportOrgEx(hdc,orig_x,ext_y-orig_y-1, NULL); /*device units*/
			SetViewportExtEx(hdc, maxSize, -maxSize, NULL); /*device units*/
  /*
			sprintf(mess, "Viewport origin x is %d y is %d\nExtent x is %d y is %d.\n\nClick to continue.", 
									orig_x,ext_y-orig_y-1,maxSize,-maxSize);
			MessageBox( hDlg, mess, "Geometry: Window Size", MB_ICONINFORMATION );
			*/
//			EnableOpenGL (hDlg, &hdc, &hRC1);
//			hRC1 = wglCreateContext( hdc );
//			wglMakeCurrent( hdc, hRC1 );
//			glViewport (drawSize.left,drawSize.top,urx,ury);
//			glViewport (0,0,urx, ury);
//			glMatrixMode (GL_PROJECTION);
//			glLoadIdentity();
//			gluOrtho2D(0.0,1.0,1.0,0.0);
//			glutMouseFunc(Mouse);
			
			   
			radius = (int) ((urx-llx)/20);
			SetDlgItemInt(hDlg,IDC_MINX,llx,TRUE);
			SetDlgItemInt(hDlg,IDC_MINY,lly,TRUE);
			SetDlgItemInt(hDlg,IDC_MAXX,urx,TRUE);
			SetDlgItemInt(hDlg,IDC_MAXY,ury,TRUE);
			SetDlgItemInt(hDlg,IDC_GRID,gridSp,TRUE);
			ScaleX=Scale(urx,llx);
			ScaleY=Scale(ury,lly);
			CheckRadioButton(hDlg, DD_J1, DD_HP, DD_J1);
			CheckRadioButton(hDlg, DD_GRON, DD_GROFF, DD_GROFF);
 
			if(jp) 
				free(jp);
			if(pp) 
				free(pp);

			jp = (Joint *) malloc(sizeof(Joint) * estNumJts);
			pp = (DPoint *) malloc(sizeof(DPoint) * estNumPts);

			nLines=0; nPoints=0; nfp=0; nmp=0; nlp=0; nhp=0;
			tool=0; type=1; inside = TRUE;

//   if (grid == TRUE)
//      SendMessage(hDlg, WM_COMMAND, DD_GRON, 0L);

			//ReleaseDC(hDlg, hdc);

}  /* close handleInit() */



double Scale (int max, int min)
{
	double temp;
	temp=max-min;
	return temp;
}  /* close scale*/


void       
handleRockbolts(HWND hDlg)
{
  HPEN hCurrentPen;
   MessageBox(hDlg, "Rock bolts are not yet implemented", "Rock bolts", MB_OK); 
   type = 1; tool=5; hCurrentPen = drawPen[1]; 

}  /* close handleRockBolts() */


void 
drawGridLines(HDC hdc, int color, GRAPHICS * g)
{
   int i;

#if OPENGL
   float red,green,blue;

			glPushMatrix();
			glBegin(GL_LINES);
			glColor3f( 1.0, 0.0, 0.0 );
			glVertex2d( 0.0, 0.0);
			glVertex2d( 1.0, 1.0);
			glEnd();
  	glPopMatrix();
			SwapBuffers(hdc);
#endif

					SelectObject(hdc, drawPen[3]); // blue
					SetROP2(hdc, R2_COPYPEN);
					
						MoveToEx(hdc, 0, 0, NULL);
						LineTo(hdc, maxSize, maxSize);

#if OPENGL
		glPushMatrix();
		glBegin(GL_LINES);
		red=g->R[color];green=g->G[color];blue=g->B[color];
		glColor3f( red, green, blue );
#endif
			   
					SelectObject(hdc, drawPen[color]); // grey
					SetROP2(hdc, R2_COPYPEN);
					i=0;
					while(i<=/*urx*/maxSize+1) {
						MoveToEx(hdc, i, 0, NULL);
						LineTo(hdc, i, maxSize);

#if OPENGL
			glVertex2d( (double)i/(double)maxSize, (double)lly/(double)maxSize);
  	glVertex2d( (double)i/(double)maxSize, (double)ury/(double)maxSize);
#endif

						i += (int)((gridSp*maxSize/ScaleX)+0.5);
					}
					i=/*lly*/0;
					while(i<=/*ury*/maxSize+1) {
						MoveToEx(hdc, 0, i, NULL);
						LineTo(hdc, maxSize, i);
#if OPENGL
			glVertex2d( (double)llx/(double)maxSize,(double)i/(double)maxSize);
		 glVertex2d( (double)urx/(double)maxSize,(double)i/(double)maxSize);
#endif
						i += (int)((gridSp*maxSize/ScaleY)+0.5);
					}

#if OPENGL
		glEnd();
 	glPopMatrix();
 	SwapBuffers(hdc);
#endif

} /* close drawGridLines() */


static void
drawJoints(HDC hdc, Joint * jp, int nLines, GRAPHICS * g)
{
   int i,j;
  /* This is just a dummy pointer for traversing.
   */
   JOINTLIST * ptr;
   Joint * jtmp;

#if OPENGL
   float red,green,blue;
   double p1x,p2x,p1y,p2y;

  	glPushMatrix();
	  glBegin(GL_LINES);
#endif

			SetROP2(hdc, R2_COPYPEN);
			for(i=0; i<nLines; i++) 
   {
						j = jp[i].type;
						SelectObject(hdc, drawPen[j]);
						MoveToEx(hdc, (int) jp[i].d1.x, (int) jp[i].d1.y, NULL);
						LineTo(hdc, (int) jp[i].d2.x, (int) jp[i].d2.y);
#if OPENGL			
	   		red=g->R[type];green=g->G[type];blue=g->B[type];
	   		glColor3f( red, green, blue );
    		p1x = jp[i].d1.x/(double)maxSize;
	   		p1y =  jp[i].d1.y/(double)maxSize;
		   	p2x =  jp[i].d2.x/(double)maxSize;
		   	p2y =  jp[i].d2.y/(double)maxSize;
		   	glVertex2d( p1x, p1y);
		   	glVertex2d( p2x, p2y);
	    	glEnd();
    		glPopMatrix();
		   	SwapBuffers(hdc);
#endif
			}

  /* Now, just for fun, redraw all those joints in purple...
   */
   dl_traverse(ptr, jointlist)
   {
      jtmp = ptr->val;
						SelectObject(hdc, drawPen[5]);
						MoveToEx(hdc, (int) jtmp->d1.x, (int) jtmp->d1.y, NULL);
						LineTo(hdc, (int) jtmp->d2.x, (int) jtmp->d2.y);
   }

}  /* close drawJoints() */



static void
drawPoints(HDC hdc, DPoint * pp, int nPoints)
{  
   int i,j;
	  POINT tri[3];

			for(i=0; i<nPoints; i++) 
   {
      SelectObject(hdc, drawPen[1]); // black
				  j = pp[i].type;
				  if(j > 1) 
         SelectObject(hdc, GetStockObject(BLACK_BRUSH));
				  else 
         SelectObject(hdc, GetStockObject(WHITE_BRUSH));

				  if( j == 0 || j == 2) 
      {
					    tri[0].x = (int) (pp[i].x);
					    tri[0].y = (int) (pp[i].y + 1.2*radius);
				    	tri[1].x = (int) (pp[i].x - 1.2*0.866*radius+.5);
			    		tri[1].y = (int) (pp[i].y - 1.2*0.5*radius);
		    			tri[2].x = (int) (pp[i].x + 1.2*0.866*radius);
	    				tri[2].y = (int) (pp[i].y - 1.2*0.5*radius);
			    		SetPolyFillMode (hdc, WINDING) ;
			    		Polygon(hdc, tri, 3);
			  	} 
      else 
      {
					    Ellipse(hdc, (int) pp[i].x-radius, (int) pp[i].y+radius, (int) pp[i].x+radius, (int) pp[i].y-radius);
			  	}
			}
} /* close drawPoints */



static void 
handleGridOn(HDC hdc, Joint * jp, int nLines, DPoint * pp, int nPoints)
{
   grid = TRUE;
  /* Ok, the deal here is that the bounding box (llx, lly etc) 
   * used to declared extern, and the values set elsewhere. 
   * We don't want to that anymore.  For now, I have hard wired
   * these in just to make something show up on the screen.
   * What needs to be done is to compute the values from the 
   * viewport and geometry scale.  For a new geometry, we can 
   * provide a scale, say from 0:100 in each direction, and
   * have a text widget where the user can overide the scale.
   */
   drawGridLines(hdc,9,g);
   drawJoints(hdc,jp,nLines,g);
   drawPoints(hdc, pp, nPoints);
}  /* close handleGridOn() */


static void 
handleGridOff(HDC hdc, Joint * jp, int nLines, DPoint * pp, int nPoints)
{
	grid = FALSE;
   drawGridLines(hdc, 0, g);
   drawJoints(hdc, jp, nLines, g);
   drawPoints(hdc, pp, nPoints);
}  /* close handleGridOff() */

static void 
handleRemoveJoints(HWND hDlg, HDC hdc, Joint * jp)
{
   if(nLines) 
   {
						SelectObject(hdc, drawPen[9]); // grey
						SetROP2(hdc, R2_COPYPEN);
						MoveToEx(hdc, (int) jp[nLines-1].d1.x, (int) jp[nLines-1].d1.y, NULL);
						LineTo(hdc, (int) jp[nLines-1].d2.x, (int) jp[nLines-1].d2.y);
						nLines--;
						if(grid) 
         SendMessage(hDlg, WM_COMMAND, DD_GRON, 0L);
						else 
         SendMessage(hDlg, WM_COMMAND, DD_GROFF, 0L);
			}

}  /* close removeJoints() */

static void 
handleRemovePoints(HWND hDlg, HDC hdc, DPoint * pp)
{
int i;
POINT tri[3];

					if(nPoints) {
						SelectObject(hdc, GetStockObject(LTGRAY_BRUSH));
						SelectObject(hdc, drawPen[9]); 
						SetROP2(hdc, R2_COPYPEN);
						i = pp[nPoints-1].type;
						if( i == 0 || i == 2) {
							if(i) nlp--;
							else nfp--;
							tri[0].x = (int) (pp[nPoints-1].x);
							tri[0].y = (int) (pp[nPoints-1].y + 1.2*radius);
							tri[1].x = (int) (pp[nPoints-1].x - 1.2*0.866*radius+.5);
							tri[1].y = (int) (pp[nPoints-1].y - 1.2*0.5*radius);
							tri[2].x = (int) (pp[nPoints-1].x + 1.2*0.866*radius);
							tri[2].y = (int) (pp[nPoints-1].y - 1.2*0.5*radius);
							SetPolyFillMode (hdc, WINDING) ;
							Polygon(hdc, tri, 3);
						} else {
							if(i-1) nhp--;
							else nmp--;
							Ellipse(hdc, (int) pp[nPoints-1].x-radius, (int) pp[nPoints-1].y+radius, (int) pp[nPoints-1].x+radius, (int) pp[nPoints-1].y-radius);
						}
						nPoints--;
                  if(grid) SendMessage(hDlg, WM_COMMAND, DD_GRON, 0L);
						else SendMessage(hDlg, WM_COMMAND, DD_GROFF, 0L);
					}
}  /* close handleInit() */


void
handleCancel(HWND hDlg)
{
   SetClassLong(hDlg, GCL_HCURSOR, (long)LoadCursor(NULL, IDC_ARROW));

			ReleaseDC(GetDlgItem(hDlg,IDC_DRAWSPACE),hdc);
			ReleaseDC(hDlg, hdc);
			EndDialog (hDlg, 0);
} /* close handleCancel() */


int 
handleSave(HWND hDlg)
{
	  int i;
	  int numflushed;
	  OPENFILENAME ofn;
	  OFSTRUCT of;
	  FILE *fp;
	  HFILE hFile;

			LPCTSTR szFilter[] = {
								"Geometry files (*.geo)\0*.geo\0All files (*.*)\0*.*\0\0"};
					strcpy(filepath.oldpath, filepath.gpath);
    /*  Need to make a call to the filebrowser. 
     */
					filepath.gpath[0] = '\0';
					memset( &ofn, 0, sizeof(OPENFILENAME) );
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFilter = szFilter[0];
					ofn.lpstrFile = (LPTSTR) filepath.gpath;
					ofn.nMaxFile = 256;
					ofn.lpstrFileTitle = (LPTSTR) filepath.gfile;
					ofn.lpstrDefExt = "geo";
					ofn.nMaxFileTitle = 256;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
					
					numflushed = _flushall();  // flush buffer

					if( !GetSaveFileName(&ofn) ) {
						strcpy(filepath.gpath, filepath.oldpath);
						//break;  // user pressed cancel
     /* Cannot have a break here. 
      * WARNING!!!!  I am assuming this should 
      * pass back as false.
      */
      return FALSE;
					} else { // open file and save data
						// if it exists already, delete it
						if(-1 != OpenFile(filepath.gpath, &of, OF_WRITE) )
								hFile = OpenFile(filepath.gpath, &of, OF_DELETE); 
						if(-1 == (hFile = OpenFile(filepath.gpath, &of, OF_CREATE)) ) {
								MessageBox(NULL, "Error: Cannot create file", "ERROR", MB_OK | MB_ICONINFORMATION);
								return 0;
						}
						fp = fopen(filepath.gpath, "w+");
						
						fprintf(fp, "0.01\r\n%d 0\r\n0\r\n0\r\n%d\r\n%d\r\n%d\r\n%d\r\n", nLines, nfp, nlp, nmp, nhp);
	
//			MessageBox(NULL, "Opened file", "Output started", MB_OK);

						for(i=0; i<nLines; i++) {
							fprintf(fp, "%.0lf %.0lf %.0lf %.0lf %d\r\n", jp[i].d1.x, jp[i].d1.y,
								jp[i].d2.x, jp[i].d2.y, jp[i].type);
						}
						for(i=0; i<nfp; i++) {
							fprintf(fp, "%.0lf %.0lf %.0lf %.0lf\r\n", pp[i].x, pp[i].y,
								pp[i].x, pp[i].y);
						}
						for(i=nfp; i<nPoints; i++) {
							fprintf(fp, "%.0lf %.0lf\r\n", pp[i].x, pp[i].y);
						}
						fclose(fp);
						_lclose(hFile);
						SetClassLong(hDlg, GCL_HCURSOR, (long)LoadCursor(NULL, IDC_ARROW));
						ReleaseDC(hDlg, hdc);
						EndDialog (hDlg, 1);   // return 1: save data
						return TRUE ;
					} // end if
	  // end case ID_OK (save)

}  /* close handleSave */




static int 
changeGridSpacing(HWND hDlg,LPARAM lParam,WPARAM wParam)
{
   //int i;

   if (HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         gridSp=GetDlgItemInt(hDlg, IDC_GRID, NULL, TRUE);
      }
   }
  	return gridSp;

}  /* close changeGridSpacing() */


static void
freeJointList()
{


}  /* close freeJointList() */


static void 
freePointList()
{

}  /* close freePointList() */