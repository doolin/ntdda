/* File contains functions to handle the creation and editing
 * of geometry files for DDA.  
 */

#include "drawdialog.h"
#include "winmain.h"
#if OPENGL
#include <gl/glut.h>
#endif  

#include <assert.h>


static int gridSp=10;
static int newgridSp;
/* Bounding box.  Was set in winmain.  Needs to be set 
 * here.  See note on DD_GRON  
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
static BOLTLIST * boltlist;

static POINT  ptBegin, ptNew, ptOld;

static enum TOOLTYPE  tool;
static int type=1, inside = 0;
static int  radius, maxSize;	
static HPEN hCurrentPen;
static HBRUSH hCurrentBr;
static	char temp[80];
static HDC hdc; 
extern HPEN drawPen[10];
extern FILEPATHS filepath;

enum context {selection, joint1, point1, bolt1} context;


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
     /*
      case WM_PAINT:
         handlePaint(hDlg);
      break;
      */
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


/*
static void
initializeDrawDialog()
{



} */ /* close initializeDrawDialog() */








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


/* This produces an infinite loop and does not allow the 
 * drawing dialog box to be initialized correctly.
 * The function is necessary however, to handle window
 * expose events, etc.
 */
static void
handlePaint(HWND hDlg)
{

   MessageBox(hDlg, "blah", "blah", MB_OK);

}  /* close handlePaint() */


static void
handleLButtonDown(HWND hDlg, LPARAM lParam)
{

   //MessageBox(hDlg, "Left Button Down", NULL, MB_ICONEXCLAMATION | MB_OKCANCEL);

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
      //MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
	} 
   else 
   {
	   inside = FALSE;
   }

}  /* close handleMouseDown() */


static void 
handleMouseMove(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   switch(context)
      {
        case selection: 
           break;
        case joint1:
        case point1:
        case bolt1:

          /* Current location of cursor */
	       ptNew.x = LOWORD(lParam); 
          ptNew.y = HIWORD(lParam);

	       DPtoLP(hdc, &ptNew, 1);

	       if(ptNew.x <= maxSize && 
             ptNew.x >= 0       && 
             ptNew.y <= maxSize && 
             ptNew.y >= 0       && 
             inside) 
          {
             //rescale coordinates
		       ScaledX=(int)(ptNew.x*(ScaleX/maxSize));
		       ScaledY=(int)(ptNew.y*(ScaleY/maxSize));


             SetDlgItemInt(hDlg, DD_X, ScaledX,TRUE);
	  	       SetDlgItemInt(hDlg, DD_Y, ScaledY,TRUE);

		       SelectObject(hdc, hCurrentPen);
 		       if(wParam & MK_LBUTTON)
             {
                SetROP2(hdc, R2_NOT);
		          if(!tool) 
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
handleLButtonUp(HWND hDlg, LPARAM lParam)
{
   switch (context)
   {
      case selection:
      //ok now, 
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
     /* Now add this turkmeister to the tail of the 
      * ddadlist type jointlist...
      */
      dl_insert_b(jointlist, (void *)newjoint);
   }

  /* Now draw to screen */
 	MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
 	LineTo(hdc, ptNew.x, ptNew.y);

}  /* close addJoint() */


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


/* WARNING!!!  This function is not called, nor should it 
 * be until the appropriate drawing and radio button 
 * handling works correctly.  It is left uncommented
 * to ensure that it compiles correctly while other 
 * issues are getting worked out.
 */
static void
addBolt(HWND hDlg)
{
   BOLT * newbolt;

   if( (ptBegin.x != ptNew.x) ||  
       (ptBegin.y != ptNew.y)  )
   {
     /* Grab a new bolt.. */
      newbolt = (BOLT *)calloc(1,sizeof(BOLT));
     /* Set the endpoints */
			   newbolt->d1.x = ptBegin.x;
		 			newbolt->d1.y = ptBegin.y;
						newbolt->d2.x = ptNew.x;
	  			newbolt->d2.y = ptNew.y;
      dl_insert_b(boltlist, (void *)newbolt);
   }

  /* Now draw to screen */
 		MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
 		LineTo(hdc, ptNew.x, ptNew.y);

}  /* close addBolt() */




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

   jointlist = make_dl();
   pointlist = make_dl();
   boltlist = make_dl();

	GetWindowRect(hDlg, &winSize);
	GetWindowRect(GetDlgItem(hDlg,IDC_DRAWSPACE), &drawSize);
	hdc = GetDC(GetDlgItem(hDlg,IDC_DRAWSPACE)); 

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
			   
			radius = (int) ((urx-llx)/20);
			SetDlgItemInt(hDlg,IDC_MINX,llx,TRUE);
			SetDlgItemInt(hDlg,IDC_MINY,lly,TRUE);
			SetDlgItemInt(hDlg,IDC_MAXX,urx,TRUE);
			SetDlgItemInt(hDlg,IDC_MAXY,ury,TRUE);
			SetDlgItemInt(hDlg,IDC_GRID,gridSp,TRUE);
			ScaleX=Scale(urx,llx);
			ScaleY=Scale(ury,lly);

			CheckRadioButton(hDlg, DD_J1, DD_BOLT1, DD_J1);
			CheckRadioButton(hDlg, DD_GRON, DD_GROFF, DD_GROFF);
         CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO4, IDC_RADIO1);

			tool=0; type=1; inside = TRUE;
}  



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
					
	MoveToEx(hdc, 0, 0, NULL);
	LineTo(hdc, maxSize, maxSize);

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
   M_dl_traverse(ptr, jointlist)
   {
      jtmp = ptr->val;
      color = jtmp->type;
		SelectObject(hdc, drawPen[color]);
		MoveToEx(hdc, (int) jtmp->d1.x, (int) jtmp->d1.y, NULL);
		LineTo(hdc, (int) jtmp->d2.x, (int) jtmp->d2.y);
   }

}  /* close drawJoints() */



static void
drawPoints(HDC hdc)
{  
   POINTLIST * ptr;
   DPoint * ptmp;

  /* Draw points stored in pointlist.
   */
   M_dl_traverse(ptr, pointlist)
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
	MoveToEx(hdc, (int) jtmp->d1.x, (int) jtmp->d1.y, NULL);
	LineTo(hdc, (int) jtmp->d2.x, (int) jtmp->d2.y);

  /* Clean up. */
   free(jtmp);
   dl_delete_node(jointlist->blink);  

	if(grid) 
      SendMessage(hDlg, WM_COMMAND, DD_GRON, 0L);
	else 
      SendMessage(hDlg, WM_COMMAND, DD_GROFF, 0L);

}  /* close removeJoints() */


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
   //FILE *fp;
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

      geomstruct = initGeometrydata(); 
      transferJointlistToGeomStruct(geomstruct, jointlist); 
      transferPointlistToGeomStruct(geomstruct, pointlist);  
      transferBoltlistToGeomStruct(geomstruct, boltlist);  

      fp = fopen(filepath.gpath, "w+");
      geomstruct->dumptofile(geomstruct->this, filepath.gpath);
      fclose(fp);
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

   M_dl_traverse(ptr, jointlist)
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

   M_dl_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      free(ptmp);
   }
   dl_delete_list(pointlist);

}  /* close freePointList() */

static void 
freeBoltList()
{

   DList * ptr;
   BOLT * btmp;

   M_dl_traverse(ptr, boltlist)
   {
      btmp = ptr->val;
      free(btmp);
   }
   dl_delete_list(boltlist);

}  /* close freeBoltList() */


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
   M_dl_traverse(ptr, jointlist)
   {
      jtmp = ptr->val;

      gd->joints[i+1][1] = jtmp->d1.x;
      gd->joints[i+1][2] = jtmp->d1.y; 
      gd->joints[i+1][3] = jtmp->d2.x;
      gd->joints[i+1][4] = jtmp->d2.y;
      gd->joints[i+1][5] = jtmp->type;

      //sprintf(mess,"Joint type %d", jtmp->type);
      //MessageBox(NULL, mess, NULL, MB_OK);


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

   numpoints = dlist_length(pointlist);
   gd->nPoints = numpoints;
   gd->pointsize1 = numpoints+1;
   gd->pointsize2 = 9;
   gd->points = DoubMat2DGetMem(gd->pointsize1, gd->pointsize2);

  /* Massive kludgery here induced by the way that the 
   * points are ordered in the points array.  The order
   * and the type are not the same.  The technique is 
   * to traverse the list once for each type of point,
   * only storing one type at a time, and in the correct
   * order in the points array.  First up is FIXED:
   */
   M_dl_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == 0)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x;
         gd->points[i+1][2] = ptmp->y; 
         gd->points[i+1][3] = ptmp->x;
         gd->points[i+1][4] = ptmp->y;
         gd->nFPoints++;
         i++;
      }
   } 
  
  /* Now store LOAD points */
   M_dl_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == 2)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x;
         gd->points[i+1][2] = ptmp->y; 
         gd->nLPoints++;
         i++;
      }
   } 

  /* MEASURED points */
   M_dl_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == 1)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x;
         gd->points[i+1][2] = ptmp->y; 
         gd->nMPoints++;
         i++;
      }
   } 

  /* HOLE points */
   M_dl_traverse(ptr, pointlist)
   {
      ptmp = ptr->val;
      if (ptmp->type == 3)
      {
         gd->points[i+1][0] = ptmp->type;
         gd->points[i+1][1] = ptmp->x;
         gd->points[i+1][2] = ptmp->y; 
         gd->nHPoints++;
         i++;
      }
   } 

}  /* close transferPointlistToGeomStruct()  */


void
transferBoltlistToGeomStruct(Geometrydata * gd, 
                             BOLTLIST * boltlist)
{
   int i = 0;
   int numbolts;
   BOLTLIST * ptr;
   BOLT * btmp;

   numbolts = dlist_length(boltlist);
   if (numbolts == 0)
      return;

   gd->nBolts = numbolts;
   gd->rockboltsize1 = numbolts+1;
   gd->rockboltsize2 = 14;
   gd->rockbolts = DoubMat2DGetMem(gd->rockboltsize1, gd->rockboltsize2);

   M_dl_traverse(ptr, boltlist)
   {
      btmp = ptr->val;

      gd->rockbolts[i+1][1] = btmp->d1.x;
      gd->rockbolts[i+1][2] = btmp->d1.y; 
      gd->rockbolts[i+1][3] = btmp->d2.x;
      gd->rockbolts[i+1][4] = btmp->d2.y;
      //gd->rockbolts[i+1][5] = btmp->type;
      i++;
   } 

}  /* close transferBoltlistToGeomStruct()  */

