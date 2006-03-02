/* File contains functions to handle the creation and editing
 * of geometry files for DDA.  
 */

#include <math.h>

#include "drawdialog.h"
#include "winmain.h"
#if OPENGL
#include <gl/glut.h>
#endif  

#include "bolt.h"
#include "joint.h"

#ifdef WIN32
#pragma warning( disable : 4115 )        
#endif


/* These will probably mutate into general purpose functions.
 * They should probably be moved into a different header file,
 * and have a return value.
 */
static void transferJointlistToGeomStruct(Geometrydata *, JOINTLIST *);   
static void transferPointlistToGeomStruct(Geometrydata *, POINTLIST *); 
static void transferBoltlistToGeomStruct(Geometrydata *, Boltlist *);   


static int gridSp=10;

//static int newgridSp;


#define WMU_SETSELECTSTATE WM_USER+1
#define WMU_GETSELECTSTATE WM_USER+2


/* Bounding box.  Was set in winmain.  Needs to be set 
 * here.  See note on DD_GRON.  These are hard-wired
 * WINDOW (logical) COORDINATES!!!
 */
static int llx=0,lly=0,urx=100,ury=100;


char mess[180];
/* FIXME: It would be much nicer to have the dialog box
 * start with a grid rather than without, but doing so
 * causes a lockup.
 */
static int grid=FALSE;
/* FIXME: Describe function of these two scale variables.
 */
static double ScaleX, ScaleY;
static int ScaledX,ScaledY;
/* These are typed doubly-linked lists.  See drawdialog.h,
 * ddadlist.c and ddadlist.c
 */
static JOINTLIST * jointlist;
static POINTLIST * pointlist;
static Boltlist * boltlist;

static POINT  ptBegin, ptNew, ptOld;


static enum TOOLTYPE  tool;
static int type=1, inside = 0;
static int  radius, maxSize;	
static HPEN hCurrentPen;
static HBRUSH hCurrentBr;
static	char temp[80];
static HDC hdc; 


extern HPEN drawPen[10];


/*  Tuck this into the parameter passed by the 
 * calling function.
 */
extern Filepaths filepath;


static int changeGridSpacing(HWND,LPARAM,WPARAM);
static int handleWMCommand(HWND, LPARAM, WPARAM);
static void handleLButtonDown(HWND hDlg, LPARAM lParam);
static void handleLButtonUp(HWND hDlg, LPARAM lParam);
static void handleMouseMove(HWND, LPARAM, WPARAM);
static void handlePaint(HWND);
static void freePointList();
static void freeJointList();
static void freeBoltList();

static void drawSinglePoint(HDC, DPoint *);
static int saveData(HWND hDlg);

/* Mouse handling functions */
static void addJoint(HWND);
static void addPoint(HWND);
static void addBolt(HWND);


static enum context {selection, joint1, point1, bolt1} context;

static DPoint DPointtoPPoint(HWND hwMain, int xpos, int ypos);

static ddaboolean JointSelected = FALSE;


static ddaboolean
getSelectState()
{

   return JointSelected;

}
   
static void
setSelectState(ddaboolean state)
{

   JointSelected = state;

}


static void
handleDrawContext(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   //int state;
   HWND hbutton = (HWND)lParam;

   WORD controlid = LOWORD(wParam);

   switch (controlid)
   {
      case IDC_RADIO1:
         context = selection;
         //MessageBox(hDlg,"Checked", NULL, MB_OK);
         break;

      case IDC_RADIO2:
         context = joint1;
         //MessageBox(hDlg,"Checked", NULL, MB_OK);
         break;

      case IDC_RADIO3:
         context = point1;
         //MessageBox(hDlg,"Checked", NULL, MB_OK);
         break;

      case IDC_RADIO4:
         context = bolt1;
         //MessageBox(hDlg,"Checked", NULL, MB_OK);
         break;

      default:
         break;
   }

/*
   state = SendMessage(hbutton, BM_GETCHECK, 0, 0);

   if (state == BST_CHECKED)
   {
      MessageBox(hDlg,"Checked", NULL, MB_OK);
      SendMessage(hbutton,BM_SETSTATE, TRUE, 0);
   }
   else //  UNCHECKED
   {
      MessageBox(hDlg,"Unchecked", NULL, MB_OK);
      SendMessage(hbutton, BM_SETSTATE, FALSE, 0);
   }
   //MessageBox(hDlg,"Select not implemented", NULL, MB_OK);
*/


}  /* close handleDrawContext() */








static ddaboolean //entirely trg error here
inBoundingBox(Joint * jp, POINT p, double eps)
{
   //int flag = 1;
   double endpx1,endpy1,endpx2,endpy2,currentx,currenty; 
   endpx1 = jp->epx1;
   endpy1 = jp->epy1;
   endpx2 = jp->epx2;
   endpy2 = jp->epy2;
   currentx = p.x;
   currenty = p.y;

   if (endpx1 > endpx2)
   {
     if ((currentx < endpx2-eps) || (currentx > endpx1+eps))
         return FALSE;
   }
   else 
   {
     if ((currentx > endpx2+eps) || (currentx < endpx1-eps))
         return FALSE;
   }
   if (endpy1 > endpy2)
   {
     if ((currenty < endpy2-eps) || (currenty > endpy1+eps))
         return FALSE;
   }
   else 
   {
     if ((currenty > endpy2+eps) || (currenty < endpy1-eps))
         return FALSE;
   }
   return TRUE;

} /* close inBoundingBox */

static int //trg addition
inBoundingBoxEP(Joint * jp, POINT p)
{
   //int flag = 1;
   double eps;
   double endp1x,endp1y,endp2x,endp2y,currentx,currenty; 
   eps = 7.0;
   endp1x = jp->epx1;
   endp1y = jp->epy1;
   endp2x = jp->epx2;
   endp2y = jp->epy2;
   currentx = p.x;
   currenty = p.y;

   
   if ((currentx < endp1x+eps) && (currentx > endp1x-eps) && (currenty < endp1y+eps) && (currenty > endp1y-eps))
	   return 1;
   if ((currentx < endp2x+eps) && (currentx > endp2x-eps) && (currenty < endp2y+eps) && (currenty > endp2y-eps))
	   return 2;
   return 0;
} /* close inBoundingBoxEP */

static int //trg edit-->added BBox
closetoJoint(Joint * jp, POINT p)
{
   char message[180];
   double eps = 7.0;
   double twicearea;
   double a,b,c,d;
   double distance;

   if (!(inBoundingBox(jp, p, eps)))
   {
       sprintf(message,"outside of Bounding Box");
       //MessageBox(NULL, message, NULL, MB_OK);
       return FALSE;
   }
   a = jp->epx2 - jp->epx1;
   b = p.x  - jp->epx1;
   c = jp->epy2 - jp->epy1;
   d = p.y  - jp->epy1;
  /* we want half the determinant ad-bc */
   twicearea  = fabs( a*d - b*c );
   
  /* Since A = dh/2 */
   distance = twicearea / (jp->length);
   
   sprintf(message,"distance: %f", distance);
   //MessageBox(NULL, message, NULL, MB_OK);

   if (distance < eps)
   {
      //MessageBox(NULL, message, "Close to joint", MB_OK);
      return TRUE;
   }
   else
      return FALSE;

}  /* close closetoJoint() */


static double //trg
distBtwPoints(POINT p1, POINT p2)
{
 
 return sqrt( (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y) );
 
}


static void      //travis
//drawBlackSquare(DPoint * ptmp, int radius)
drawBlackSquare(double x, double y, int radius) {

   //POINT tri[3];
   POINT square[4];
   hCurrentBr = GetStockObject(BLACK_BRUSH);

  /* Draw a single point.  This could and probably
   * should be abstracted into a function that can 
   * be called from here and from the drawPoints
   * function.
   */
   SelectObject(hdc, hCurrentBr);

   //drawSinglePoint(hdc, newpoint);

   square[0].x = (int) (x + radius);
   square[0].y = (int) (y + radius);
   square[1].x = (int) (x + radius);
   square[1].y = (int) (y - radius);
   square[2].x = (int) (x - radius);
   square[2].y = (int) (y - radius);
   square[3].x = (int) (x - radius);
   square[3].y = (int) (y + radius);

	SetPolyFillMode (hdc, WINDING) ;
	Polygon(hdc, square, 4);
  


}  /* drawBlockBox() */


static void
drawJointHandles(Joint * jp) {

   int radius = 3; // pixels
   drawBlackSquare(jp->epx1, jp->epy1, radius);
   drawBlackSquare(jp->epx2, jp->epy2, radius);

} 


/** @todo Fix the api to not take the point struct. */
static void  //trg
drawJointHandle(Joint * jp, int num)
{
   int radius = 3; // pixels


   if (num == 1)
	   drawBlackSquare(jp->epx1, jp->epy1, radius);
   if (num == 2)
	   drawBlackSquare(jp->epx2, jp->epy2, radius);


}  /* close drawJointHandles() */


static void //trg
redrawJoint(Joint * jp)
{
  int endp1x,endp1y,endp2x,endp2y; 
   endp1x = (int)jp->epx1;
   endp1y = (int)jp->epy1;
   endp2x = (int)jp->epx2;
   endp2y = (int)jp->epy2; 
   
   MoveToEx(hdc, endp1x, endp1y, NULL);

   LineTo(hdc, endp2x, endp2y);
   if (jp->is_selected)
      drawJointHandles(jp);
}  /* close drawJoint() */



/* This produces an infinite loop and does not allow the 
 * drawing dialog box to be initialized correctly.
 * The function is necessary however, to handle window
 * expose events, etc.
 */
/*static void
handlePaint(HWND hDlg)
{
   JOINTLIST * ptr;
   Joint * jtmp;
   M_dl_traverse(ptr, jointlist)
   {
        jtmp = ptr->val;
        redrawJoint(jtmp);          
   }

}  /* close handlePaint() */

/*static void              // won't work because hdc finds no valid regions
handlePaint(HWND hDlg)
{
   HDC hdc; 
   PAINTSTRUCT ps;
   hdc = BeginPaint(hDlg, &ps );
   drawJoints(hdc);
   EndPaint(hDlg, &ps );
}  /* close handlePaint() */


static void        // this works somewhat, because i think redraw joint refers to global hdc 
handlePaint(HWND hDlg)
{ 
   HDC hdc;
   PAINTSTRUCT ps;
   JOINTLIST * ptr;
   Joint * jtmp;
   //InvalidateRect(hDlg, NULL, TRUE); //test2 no work
   hdc = BeginPaint(hDlg, &ps );
   dlist_traverse(ptr, jointlist) {

        jtmp = ptr->val;
        redrawJoint(jtmp);          
   }
   EndPaint(hDlg, &ps );
   //InvalidateRect(hDlg, NULL, TRUE); //test CRASHED!!!
}  /* close handlePaint() */


static void
handleLButtonDown(HWND hDlg, LPARAM lParam)
{

   //MessageBox(hDlg, "Left Button Down", NULL, MB_ICONEXCLAMATION | MB_OKCANCEL);
   JOINTLIST * ptr;
   Joint * jtmp;
   int i = 0;
   int j = 0;
   int num;
   char message[80];

   SetCapture(hDlg);
  /* Get the (x,y) of the pointer when the left 
   * mouse button is pressed.
   */
   ptBegin.x = LOWORD(lParam); 
   ptBegin.y = HIWORD(lParam);
   DPtoLP(hdc, &ptBegin, 1);
   switch (context)
   {
       case selection:
             if (SendMessage(hDlg,WMU_GETSELECTSTATE,0,0)) 
             {
                dlist_traverse(ptr, jointlist)
                 {
                  jtmp = ptr->val;
                  if(closetoJoint(jtmp,ptBegin))
                     j++; 
                  if (j == 0)
                      SendMessage(hDlg,WMU_SETSELECTSTATE,(WPARAM)FALSE,0);//JointSelected = FALSE;      
                            
                 }
             }
             else 
             {
                 dlist_traverse(ptr, jointlist)
                 {
                    jtmp = ptr->val;
					if (inBoundingBoxEP(jtmp,ptBegin)) {
                        num = inBoundingBoxEP(jtmp,ptBegin);
						drawJointHandle(jtmp, num);
						i++; 
					}
					else
						if(closetoJoint(jtmp,ptBegin))
							drawJointHandles(jtmp);
							JointSelected = TRUE;              
                            i++;            
                 }
                 sprintf(message,"Number of joints: %d",i);
                 //MessageBox(NULL, message, NULL, MB_OK);
             }
       case joint1:
       case point1:
       case bolt1:
           //DPtoLP(hdc, &ptBegin, 1);
           //ScreenToClient(hDlg,&ptBegin);
           if(ptBegin.x <= maxSize && 
              ptBegin.x >= 0       && 
              ptBegin.y <= maxSize && 
              ptBegin.y >= 0        ) 
           {
              ptOld = ptBegin;
	           inside = TRUE;
              //MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
	        } 
           else 
           {
	          inside = FALSE;
           }
    }
}  /* close handleMouseDown() */


static void 
handleMouseMove(HWND hDlg, LPARAM lParam, WPARAM wParam)
{

  /* Current location of cursor */
   ptNew.x = LOWORD(lParam); 
   ptNew.y = HIWORD(lParam);
   DPtoLP(hdc, &ptNew, 1);

   //rescale coordinates
   ScaledX=(int)(ptNew.x*(ScaleX/maxSize));
   ScaledY=(int)(ptNew.y*(ScaleY/maxSize));
   SetDlgItemInt(hDlg, DD_X, ScaledX,TRUE);
   SetDlgItemInt(hDlg, DD_Y, ScaledY,TRUE);


      switch(context)
      {

        case selection: 
           break;
        case joint1:
        case point1:
        case bolt1:


	       if(ptNew.x <= maxSize && 
             ptNew.x >= 0       && 
             ptNew.y <= maxSize && 
             ptNew.y >= 0       && 
             inside) 
          {

		       SelectObject(hdc, hCurrentPen);
 		       if(wParam & MK_LBUTTON)
             {
                SetROP2(hdc, R2_NOT);  //R2_NOT operates on pixels bitwise pen/dest boot each other 
		          if(tool == joint) 
                {
                   MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
			          LineTo(hdc, ptOld.x, ptOld.y);
				       MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
				       LineTo(hdc, ptNew.x, ptNew.y);
                }
			       ptOld = ptNew;
             }

	       } 
          else 
          {  // outside drawing area
			   	  // set dialog item text to blank
             temp[0] = '\0';
		       SetDlgItemText(hDlg, DD_X, temp);
		       SetDlgItemText(hDlg, DD_Y, temp);
		       ptNew = ptOld;
	       }
			   // ReleaseDC(hDlg, hdc);
        }

}  /* close handleMouseMove() */




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
		newjoint->epx1 = ptBegin.x;
		newjoint->epy1 = ptBegin.y;
		newjoint->epx2 = ptNew.x;
	  	newjoint->epy2 = ptNew.y;
		newjoint->type = type;

     /* Let's compute the length by distance formula future reference. */
      newjoint->length = sqrt( (ptBegin.x - ptNew.x)*(ptBegin.x - ptNew.x) + (ptBegin.y - ptNew.y)*(ptBegin.y - ptNew.y) );

     /* When a joint is first drawn, require it to not be selected */
      newjoint->is_selected = FALSE;

     /* Now add this turkmeister to the tail of the 
      * ddadlist type jointlist...
      */
      dl_insert_b(jointlist, (void *)newjoint);
   }

  /* Now draw to screen */
 	MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
 	LineTo(hdc, ptNew.x, ptNew.y);

}  /* close addJoint() */




/* WARNING!!!  This function is not called, nor should it 
 * be until the appropriate drawing and radio button 
 * handling works correctly.  It is left uncommented
 * to ensure that it compiles correctly while other 
 * issues are getting worked out.
 */
static void
addBolt(HWND hDlg)
{
   Bolt * newbolt;

   if( (ptBegin.x != ptNew.x) ||  
       (ptBegin.y != ptNew.y)  ) {

      newbolt = bolt_new();
      bolt_set_endpoints(newbolt,ptBegin.x, ptBegin.y,ptNew.x,ptNew.y);
      boltlist_append(boltlist,newbolt);
   }

  /* Now draw to screen */
 	MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
 	LineTo(hdc, ptNew.x, ptNew.y);

}




/* Also sets radius variable used to draw points.
 */
static void
handleInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
   RECT winSize;
	RECT drawSize;
	int orig_x,orig_y,ext_x,ext_y;
   
   context = selection;

   

/* Start test code for dialog param passing */
/*
   FILE * inittest;

   inittest = fopen("drawdlg.txt","w");

   switch(lParam)
   {
      case 3:
         fprintf(inittest,"lParam %d passed in from winmain\n");
         break;

      default:
         fprintf(inittest,"lParam %d passed in from winmain\n");
         break;
   }

   fclose(inittest);
*/
/*  End test code for dialog param passing */

   jointlist = dlist_new();
   pointlist = dlist_new();
   //boltlist = make_dl();
   boltlist = boltlist_new();

	GetWindowRect(hDlg, &winSize);
	GetWindowRect(GetDlgItem(hDlg,IDC_DRAWSPACE), &drawSize);
	hdc = GetDC(GetDlgItem(hDlg,IDC_DRAWSPACE));  // i think this is DEADLY wrong  TRG  getDC does not "validate"

	SetClassLong(GetDlgItem(hDlg,IDC_DRAWSPACE), GCL_HCURSOR, (long)LoadCursor(NULL,"IDC_CROSS "));

	hCurrentPen = drawPen[1];  // black
		
	SetMapMode(hdc, MM_ISOTROPIC);
						
  /*
	sprintf(mess, "Dialog top is %d bottom is %d\nDraw top is %d bottom is %d.\n\nClick to continue.", 
	winSize.top,winSize.bottom,drawSize.top,drawSize.bottom);
	MessageBox( hDlg, mess, "Geometry: Window Size", MB_ICONINFORMATION );
	*/
	
	orig_x = drawSize.left-winSize.left;
	orig_y = drawSize.top-winSize.top;
	ext_x = drawSize.right-drawSize.left;
	ext_y = drawSize.bottom-drawSize.top;

			
	SetWindowOrgEx(hdc,orig_x,orig_y, NULL);
	SetWindowExtEx(hdc,ext_x,ext_y,NULL);

			
   // want square area to draw in
   maxSize = ext_x-orig_x; /*> ury-lly ? urx : ury;*/

	if ( (ury-lly) > maxSize) 
      maxSize = ext_y - orig_y; 


	SetViewportOrgEx(hdc,orig_x,ext_y-orig_y-1, NULL); /*device units*/
	SetViewportExtEx(hdc, maxSize, -maxSize, NULL); /*device units*/

  /*
	sprintf(mess, "Viewport origin x is %d y is %d\nExtent x is %d y is %d.\n\nClick to continue.", 
	orig_x,ext_y-orig_y-1,maxSize,-maxSize);
	MessageBox( hDlg, mess, "Geometry: Window Size", MB_ICONINFORMATION );
   */
			   
	radius = (int) ((urx-llx)/20);
	SetDlgItemInt(hDlg,IDC_MINX,llx,TRUE);
	SetDlgItemInt(hDlg,IDC_MINY,lly,TRUE);
	SetDlgItemInt(hDlg,IDC_MAXX,urx,TRUE);
	SetDlgItemInt(hDlg,IDC_MAXY,ury,TRUE);
	SetDlgItemInt(hDlg,IDC_GRID,gridSp,TRUE);
	ScaleX = Scale(urx,llx);
	ScaleY = Scale(ury,lly);

			CheckRadioButton(hDlg, DD_J1, DD_BOLT1, DD_J1);
			CheckRadioButton(hDlg, DD_GRON, DD_GROFF, DD_GROFF);
         CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO4, IDC_RADIO1);

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
handleRockBolts(HWND hDlg)
{
   //HPEN hCurrentPen;
   //MessageBox(hDlg, "Rock bolts are not yet implemented", "Rock bolts", MB_OK); 
   //type = 1; tool=5; hCurrentPen = drawPen[1]; 

}  /* close handleRockBolts() */


void 
drawGridLines(HDC hdc, int color)
{
   int i;

	SelectObject(hdc, drawPen[3]); // blue
	SetROP2(hdc, R2_COPYPEN);
					
	//MoveToEx(hdc, 0, 0, NULL);
	//LineTo(hdc, maxSize, maxSize);

	SelectObject(hdc, drawPen[color]); // grey
	SetROP2(hdc, R2_COPYPEN);
	i=0;

	while(i<=/*urx*/maxSize+1) 
   {
	   MoveToEx(hdc, i, 0, NULL);
	   LineTo(hdc, i, maxSize);
		i += (int)((gridSp*maxSize/ScaleX)+0.5);
	}
	i=/*lly*/0;

   while(i<=/*ury*/maxSize+1) 
   {
      MoveToEx(hdc, 0, i, NULL);
      LineTo(hdc, maxSize, i);
      i += (int)((gridSp*maxSize/ScaleY)+0.5);
   }

} /* close drawGridLines() */


static void
drawJoints(HDC hdc)
{
   int color;
   JOINTLIST * ptr;
   Joint * jtmp;

  /* Draw joints stored in jointlist.
   */
   dlist_traverse(ptr, jointlist)
   {
      jtmp = ptr->val;
      color = jtmp->type;
		SelectObject(hdc, drawPen[color]);
		MoveToEx(hdc, (int) jtmp->epx1, (int) jtmp->epy1, NULL);
		LineTo(hdc, (int) jtmp->epx2, (int) jtmp->epy2);
   }

}  /* close drawJoints() */




static void 
handleGridOn(HDC hdc)
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
   drawGridLines(hdc,9);
   drawJoints(hdc);
   drawPoints(hdc);
}  /* close handleGridOn() */


static void 
handleGridOff(HDC hdc)
{
	grid = FALSE;
   drawGridLines(hdc, 0);
   drawJoints(hdc);
   drawPoints(hdc);
}  /* close handleGridOff() */


static void 
handleRemoveJoints(HWND hDlg, HDC hdc)
{
   Joint * jtmp;
 
   if (jointlist->blink == jointlist)
      return; // No mo' joints...

   jtmp = jointlist->blink->val;

	SelectObject(hdc, drawPen[0]); // 0 is white
	SetROP2(hdc, R2_COPYPEN);
	MoveToEx(hdc, (int) jtmp->epx1, (int) jtmp->epy1, NULL);
	LineTo(hdc, (int) jtmp->epx2, (int) jtmp->epy2);

  /* Clean up. */
   free(jtmp);
   dl_delete_node(jointlist->blink);  

	if(grid) 
      SendMessage(hDlg, WM_COMMAND, DD_GRON, 0L);
	else 
      SendMessage(hDlg, WM_COMMAND, DD_GROFF, 0L);

}  /* close removeJoints() */



void
handleCancel(HWND hDlg)
{
  /* FIXME: Problem with these two calls: seg faults.
   */
   //freePointList();
   //freeJointList();
   SetClassLong(hDlg, GCL_HCURSOR, (long)LoadCursor(NULL, IDC_ARROW));
	ReleaseDC(GetDlgItem(hDlg,IDC_DRAWSPACE),hdc);
	ReleaseDC(hDlg, hdc);
	EndDialog (hDlg, 0);
} /* close handleCancel() */


int 
handleSave(HWND hDlg)
{

   int numflushed;
   OPENFILENAME ofn;
   OFSTRUCT of;
   FILE *fp;
   HFILE hFile;
   Geometrydata * geomstruct;

   LPCTSTR szFilter[] = {
             "Geometry files (*.geo)\0*.geo\0All files (*.*)\0*.*\0\0"};

   fileBrowse(hDlg, &ofn, szFilter, filepath.gpath, filepath.gfile, "geo");

   numflushed = _flushall();  // flush buffer

   if( !GetSaveFileName(&ofn) ) 
   {
		strcpy(filepath.gpath, filepath.oldpath);
		//break;  // user pressed cancel
     /* Cannot have a break here. 
      * WARNING!!!!  I am assuming this should 
      * pass back as false.
      */
      return FALSE;
			} 
   else 
   {  /* Open file and save data.
		 * If it exists already, delete it.
       */
		if(-1 != OpenFile(filepath.gpath, &of, OF_WRITE) )
	 		hFile = OpenFile(filepath.gpath, &of, OF_DELETE); 

		if(-1 == (hFile = OpenFile(filepath.gpath, &of, OF_CREATE)) )
		{
         MessageBox(NULL, "Error: Cannot create file", "ERROR", MB_OK | MB_ICONINFORMATION);
            return 0;
      }

      geomstruct = gdata_new();


      transferJointlistToGeomStruct(geomstruct, jointlist); 
      transferPointlistToGeomStruct(geomstruct, pointlist);  
      transferBoltlistToGeomStruct(geomstruct, boltlist);  


      //fp = fopen(filepath.gpath, "w+"); //Disabled by Roozbeh
	  //geomstruct->dumptofile(geomstruct, fprintf, fp); //Disabled by Roozbeh
      geomstruct->dumptofile(geomstruct, fprintf, filepath.gpath);
      //fclose(fp); //Disabled by Roozbeh
      gdata_delete(geomstruct); 

     /* FIXME: These functions are segfaulting. */
      //freeJointList(); 
      //freePointList(); 
      //freeBoltList();  

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
   DList * ptr;
   Joint * jtmp;

   dlist_traverse(ptr, jointlist)
   {
      jtmp = ptr->val;
      free(jtmp);
   }
   dl_delete_list(jointlist);

}  /* close freeJointList() */


static void 
freePointList()
{

   DList * ptr;
   DPoint * ptmp;

   dlist_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      free(ptmp);
   }
   dl_delete_list(pointlist);

}  /* close freePointList() */



static void
drawSinglePoint(HDC hdc, DPoint * ptmp)
{

   POINT tri[3];

	  if( (ptmp->type == 0) || 
       (ptmp->type == 2)  )
   {
		    tri[0].x = (int) (ptmp->x);
		    tri[0].y = (int) (ptmp->y + 1.2*radius);
	    	tri[1].x = (int) (ptmp->x - 1.2*0.866*radius+.5);
    		tri[1].y = (int) (ptmp->y - 1.2*0.5*radius);
   			tri[2].x = (int) (ptmp->x + 1.2*0.866*radius);
  				tri[2].y = (int) (ptmp->y - 1.2*0.5*radius);
    		SetPolyFillMode (hdc, WINDING) ;
    		Polygon(hdc, tri, 3);
  	} 
   else 
   {
		    Ellipse(hdc, (int) ptmp->x-radius, (int) ptmp->y+radius, 
                   (int) ptmp->x+radius, (int) ptmp->y-radius);
  	}

}  /* close drawSinglePoint() */


static void 
handleRemovePoints(HWND hDlg, HDC hdc)
{
   DPoint * ptmp;
 
   if (pointlist->blink == pointlist)
      return; // No mo' points...

   ptmp = pointlist->blink->val;

   SelectObject(hdc, GetStockObject(WHITE_BRUSH));
			SelectObject(hdc, drawPen[0]); 
			SetROP2(hdc, R2_COPYPEN);

   drawSinglePoint(hdc, ptmp);

  /* Clean up. */
   free(ptmp);
   dl_delete_node(pointlist->blink);  
   
   if(grid) 
      SendMessage(hDlg, WM_COMMAND, DD_GRON, 0L);
	else 
      SendMessage(hDlg, WM_COMMAND, DD_GROFF, 0L);

}  /* close handleRemovePoint() */



static void
drawPoints(HDC hdc)
{  
   POINTLIST * ptr;
   DPoint * ptmp;

  /* Draw points stored in pointlist.
   */
   dlist_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
		SelectObject(hdc, drawPen[1]);
     /* Select brushes here... first check for fixed 
      * or measured...
      */
      if ( (ptmp->type == 0) ||
           (ptmp->type == 1)  )
         hCurrentBr = GetStockObject(WHITE_BRUSH);
      else /* load or hole */
         hCurrentBr = GetStockObject(BLACK_BRUSH);

      SelectObject(hdc, hCurrentBr);
      drawSinglePoint(hdc, ptmp);
   }

} /* close drawPoints */



static void 
addPoint(HWND hDlg)
{
   DPoint * newpoint;

   newpoint = (DPoint *)calloc(1,sizeof(DPoint));

   newpoint->x = ptNew.x;
   newpoint->y = ptNew.y;
	newpoint->type = type;
   dl_insert_b(pointlist, (void *)newpoint);

   if(tool==loadpoint || tool==holepoint) 
      hCurrentBr = GetStockObject(BLACK_BRUSH);
   else 
      hCurrentBr = GetStockObject(WHITE_BRUSH);

  /* Draw a single point.  This could and probably
   * should be abstracted into a function that can 
   * be called from here and from the drawPoints
   * function.
   */
   SelectObject(hdc, hCurrentBr);

   drawSinglePoint(hdc, newpoint);

}  /* close addPoint() */



void
transferJointlistToGeomStruct(Geometrydata * gd, 
                              JOINTLIST * jointlist)
{
   int i = 0;
   int numjoints;
   JOINTLIST * ptr;
   Joint * jtmp;
  /* just in case... */
   numjoints = dlist_length(jointlist);
   gd->nJoints = numjoints;
   gd->jointsize1 = numjoints+1;
   gd->jointsize2 = 6;
   gd->joints = DoubMat2DGetMem(gd->jointsize1, gd->jointsize2);

  /* Draw joints stored in jointlist.
   */
   dlist_traverse(ptr, jointlist)
   {
      jtmp = ptr->val;

      gd->joints[i+1][1] = jtmp->epx1*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
      gd->joints[i+1][2] = jtmp->epy1*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
      gd->joints[i+1][3] = jtmp->epx2*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
      gd->joints[i+1][4] = jtmp->epy2*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
      gd->joints[i+1][5] = jtmp->type;
      i++;
   } 


}  /* close  transferJointlistToGeomStruct() */


void
transferPointlistToGeomStruct(Geometrydata * gd, 
                              POINTLIST * pointlist)
{
  /* WARNING!!!  The counter i is carried across four
   * loops for cumulative indexing.
   */
   int i = 0;
   int numpoints;
   POINTLIST * ptr;
   DPoint * ptmp;

  /* The edge-node distance probably ought to 
   * be set as a part of the CAD scaling.
   */
   gd->e00 = 0.01;
   // mmm: this should probably be set somewhere else, but I don't know where
   // is there a file that contains hardwired parameters?
   gd->maxFixedPointsPerFixedLine = 100; // hardwired
   numpoints = dlist_length(pointlist);
   gd->nPoints = numpoints;
//   gd->pointsize1 = ((gd->nFPoints*(gd->maxFixedPointsPerFixedLine+1))+gd->nLPoints+gd->nMPoints+gd->nHPoints+1)+1; //Diabled by Roozbeh
   gd->pointsize1 = numpoints+1; //Added By Roozbeh
   gd->pointsize2 = 9;
   gd->points = DoubMat2DGetMem(gd->pointsize1, gd->pointsize2);

  /* Massive kludgery here induced by the way that the 
   * points are ordered in the points array.  The order
   * and the type are not the same.  The technique is 
   * to traverse the list once for each type of point,
   * only storing one type at a time, and in the correct
   * order in the points array.  First up is FIXED:
   */
   dlist_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == 0)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->points[i+1][2] = ptmp->y*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->points[i+1][3] = ptmp->x*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->points[i+1][4] = ptmp->y*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->nFPoints++;
         i++;
      }
   } 
  
  /* Now store LOAD points */
   dlist_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == 2)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->points[i+1][2] = ptmp->y*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->nLPoints++;
         i++;
      }
   } 

  /* MEASURED points */
   dlist_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == 1)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->points[i+1][2] = ptmp->y*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->nMPoints++;
         i++;
      }
   } 

  /* HOLE points */
   dlist_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == 3)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->points[i+1][2] = ptmp->y*(ScaleX/maxSize); //Added (ScaleX/maxSize) term by Roozbeh
         gd->nHPoints++;
         i++;
      }
   } 

}  /* close transferPointlistToGeomStruct()  */


void
transferBoltlistToGeomStruct(Geometrydata * gd, 
                             Boltlist * boltlist) {

   int i = 0;
   int numbolts;

   numbolts = boltlist_length(boltlist);

   if (numbolts == 0) {
      return;
   }

   /*
   gd->nBolts = numbolts;
   gd->rockboltsize1 = numbolts+1;
   gd->rockboltsize2 = 14;
   gd->rockbolts = DoubMat2DGetMem(gd->rockboltsize1, gd->rockboltsize2);
   */

   gdata_rockbolt_init(gd, numbolts);
   boltlist_get_array(boltlist,gd->rockbolts);


} 



/* We really need to get the physical coordinates from the 
 * device coordinates.
 */
static DPoint
DPointtoPPoint(HWND hwMain, int xpos, int ypos)
{
  /* Convenience variables set from Graphics struct. */
   double scale, offsetx, offsety;
  /* The local device context.  We will set this and release 
   * in this function.
   */
   HDC hdc;
  /* viewport rectangle */
   RECT vprect;
  /* Viewport origin */
   POINT vporg, worg;
  /* Coordinates of physical point */
   DPoint p;

  /* Coordinates of an arbitrary point in the logical
   * (window) coordinate system.
   */
   double xwindow, ywindow;
  /* Origin of logical coordinate system */
   int xwindoworg, ywindoworg;
  /* Size of logical system based on client area */
   int xwindowext, ywindowext;
  /* Coordinates of arbitrary device point, i.e.,
   * a cursor location, etc.
   */
   int xviewport, yviewport;
  /* Size of device based on client area */
   int xviewportext, yviewportext;
  /* Origin of viewport (device) coordinates. */
   int xviewportorg, yviewportorg;

  /* Dummy variables for now. */
   //double scale, offsetx, offsety; 

  /* Segfaults if there is nothing to look at. 
   * FIXME: Add some error code in here to keep track
   * of whats going on.
   */
   //if (geomdata == NULL)
   //   return 0;


/* g is a global Geometrydata * in winmain.c.  scale and offsets
 * should be hardwired in the drawdialog for now.
 */
  /* Transfer to local variables for convience */
  /*   scale = g->scale;
   offsetx = (double)g->offset.x;
   offsety = (double)g->offset.y;
   */

  /* Not the recommended way to do this.  Note the
   * DC is released after we are done.
   */
   hdc = GetDC(hwMain);

  /* Find device origin */
   GetViewportOrgEx(hdc,&vporg);
   xviewportorg = vporg.x;
   yviewportorg = vporg.y;

  /* Find the origin of the logical coords. */
   GetWindowOrgEx(hdc, &worg);
   xwindoworg = worg.x;
   ywindoworg = worg.y;

  /* Grab the window size */
   GetClientRect(hwMain, &vprect);
   xviewportext = vprect.right;
   yviewportext = vprect.bottom;

  /* Convert the window size to logical units. */
   DPtoLP(hdc, (LPPOINT) &vprect, 2);
   xwindowext = vprect.right;
   ywindowext = -vprect.top;  

  /* For this particular function, I need the current
   * cursor position.
   */
   xviewport = xpos;
   yviewport = ypos;

  /* Translate from device to logical coordinates.
   * See discussion on p. 259, Rector & Newcomer.
   */
   xwindow = (xviewport-xviewportorg)*(xwindowext/xviewportext) + xwindoworg;
   ywindow = (yviewport-yviewportorg)*(ywindowext/yviewportext) + ywindoworg;

  /* (x,y) in physical coordinates... */
   p.x = (xwindow - offsetx)/scale;
   p.y = (ywindow - offsety)/scale;


   ReleaseDC(hwMain, hdc);

   return p;

}  /* close DPtoPP() */





static void
handleLButtonUp(HWND hDlg, LPARAM lParam)
{
	
   
   DDAPoint p;

  /* Current location of cursor */
   p.x = LOWORD(lParam); 
   p.y = HIWORD(lParam);


   ptNew.x = LOWORD(lParam); ptNew.y = HIWORD(lParam);
	DPtoLP(hdc, &ptNew, 1);


   switch (context)
   {
 
      case selection:

         

         break;




      case joint1:
      case point1:
      case bolt1:

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

		      if(tool==joint) 
               addJoint(hDlg);
            else if (joint<tool && tool<bolt) 
               addPoint(hDlg);
            else if (tool==bolt)
               handleRockBolts(hDlg);
            else /* catch an exception... later ;) */
               ;
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
	      
         //			ReleaseDC(hDlg, hdc);
   }
   ReleaseCapture();
}  /* close handleMouseUp() */



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
        /*type = 4; tool = bolt;*/
         handleRockBolts(hDlg);
         break;

	   case DD_GRON:
         handleGridOn(hdc);
	      break;

	  	case DD_GROFF:
	     	handleGridOff(hdc);
	    	break;

	   case DD_REMJ:
         handleRemoveJoints(hDlg,hdc);
 			break;

  		case DD_REMP:
         handleRemovePoints(hDlg,hdc);
 			break;

      //case IDC_DRAWCONTEXT:
         //handleDrawContext(hDlg,lParam,wParam);
         //break;
      case IDC_RADIO1:
      case IDC_RADIO2:
      case IDC_RADIO3:
      case IDC_RADIO4:
         handleDrawContext(hDlg, lParam, wParam);
         break;

     /* Handles changing grid spacing? 
      * TODO: Collapse this into a single callable function. 
      */
	 	case IDC_GRID:
	     	if (grid)
	      { 
            handleGridOff(hdc);
	         grid=TRUE;
	     	}			
		  	gridSp=changeGridSpacing(hDlg,lParam,wParam);

		   if (grid)
		  	{ 
		    	handleGridOn(hdc);			
	  		}
 	   	break;

	 	case IDCANCEL:
	   	handleCancel(hDlg);
		   return FALSE;

      case IDOK:
        	return(handleSave(hDlg));

     /* TODO: Add a default case with appropriate action.
      */

   }  /* end switch wParam */

   return FALSE;

}  /* close handleWMCommand() */



BOOL CALLBACK 
DrawDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) 
{

  	switch (iMessage)
  	{
      case WM_INITDIALOG:
	      handleInit(hDlg, wParam, lParam);
         break;

     /* FIXME: handlePaint does not work correctly.   
      * Needs to be fixed before shipping.  As a test 
      * case, cover, then uncover the dialog. 
      */
     
      case WM_PAINT:
        handlePaint(hDlg);
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

      case WMU_GETSELECTSTATE:
         return getSelectState();

      case WMU_SETSELECTSTATE:
         setSelectState((ddaboolean)wParam);
         break;
   
     /* As usual, add a destroy handler if this gets long */
      case WM_DESTROY:
        /* Free up all allocated memory. */
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


