
#define STRICT
#include<windows.h>
#include<stdio.h>
#include"geomdlg.h"
#include"ddamemory.h"
#include "interface.h"



extern FILEPATHS filepath;

static int nj = 3, np, nfp = 3, nmp = 0, nlp = 0, nhp = 0;
static int numbolts = 0;
static Joint *joints, *jOld;
static DPoint *points, *pOld;
static double ** bolts, ** oldbolts;
static int jindex = 0;
static int pindex = 0;
static int boltindex = 0;

static char subWinTitle[40];
static HFILE hFile;
static OFSTRUCT of;
static FILE *fp;
static OPENFILENAME ofn;
static HWND hScroll;
static HWND hEditCtrl;
static SCROLLINFO scrollinfo;
static Geometrydata * gd;
static char temp[80];
static char ptypes[] = "FMLH";
static char *numlist[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", \
        "12", "13", "14", "15", "16", "17", "18", "19", "20"};

/*  This duplicates the definition given in dda,c for the same function,
 * but trying to get it to work in dda.h produces continual errors.
 * Fix later.
 */
void fileBrowse(HWND, OPENFILENAME *, LPCTSTR *, char *, char *, char *);
static BOOLEAN handleInit(HWND);
static BOOLEAN handleWMCommand(HWND, LPARAM, WPARAM);
static BOOLEAN handleVScroll(HWND, LPARAM, WPARAM);
static void loadDefaults();
static void loadFromFile1(HWND);
static void loadFromFile2(HWND);
static BOOLEAN setDialogValues(HWND);
static BOOLEAN saveData();
static void changeNumberOfJoints(HWND,LPARAM,WPARAM);
static void changeNumberOfPoints(HWND,LPARAM,WPARAM);
static void changeNumberOfBolts(HWND,LPARAM,WPARAM);

static void changePointRow(HWND,LPARAM,WPARAM);

/* FIXME: Roll these up into one function similar
 * to how load points are handled in the analysis
 * dialog.
 */
static void changeJointRow1(HWND,LPARAM,WPARAM);
static void changeJointRow2(HWND,LPARAM,WPARAM);
static void changeJointRow3(HWND,LPARAM,WPARAM);
static void changeJointRow4(HWND,LPARAM,WPARAM);
static void changeJointRow5(HWND,LPARAM,WPARAM);


static void changeBoltProperties(HWND,LPARAM,WPARAM,int);
static void handleBoltNumber(HWND,LPARAM,WPARAM);
static void enableBoltBoxes(HWND, BOOLEAN);
static void initBoltBoxes(HWND hDlg);
static void setBoltBoxes(HWND hDlg, int);

/* Code to handle the geometry dialog box.  This is not particularly
 * clever as it relies on the file to edit being stored in an external 
 * variable.   Then it has to read in the geometry data.  A better way would 
 * be to pass in the geometry data directly.  Or at least to use the 
 * previously written geometry file data reader.
 */

BOOL CALLBACK  
GeomDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
   switch (iMessage)
   {
      case WM_INITDIALOG :
         return(handleInit(hDlg));     
    
      case WM_COMMAND:
         return(handleWMCommand(hDlg, lParam, wParam));

      case WM_VSCROLL:
         return(handleVScroll(hDlg, lParam, wParam));

   } // end switch (imessage)

   return FALSE ;

}  // end GeomDlgProc



static BOOLEAN
handleInit(HWND hDlg)
{

   if(filepath.gfile[0] != '\0') 
      loadFromFile2(hDlg);
   else 
      loadDefaults();

   return(setDialogValues(hDlg));

}  /* close handleInit() */


/* 
 * Night time, in the switching yard...
 */

static BOOLEAN
handleWMCommand(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   switch (LOWORD(wParam))
   {
      case GD_SAVE:
         return(saveData(hDlg));

      case IDCANCEL:
         EndDialog (hDlg, 0);   // return 0: don't save data
         return TRUE;

      case GD_NJOI:
         changeNumberOfJoints(hDlg, lParam,wParam);
         break;

      case GD_NFP:
      case GD_NMP:
      case GD_NLP:
      case GD_NHP:
         changeNumberOfPoints(hDlg,lParam,wParam);
         break;

      case GD_J1X1: 
      case GD_J1Y1: 
      case GD_J1X2: 
      case GD_J1Y2: 
      case GD_J1T:
         changeJointRow1(hDlg,lParam,wParam);
         break;

      case GD_J2X1: 
      case GD_J2Y1: 
      case GD_J2X2: 
      case GD_J2Y2: 
      case GD_J2T:
         changeJointRow2(hDlg,lParam,wParam);
         break;

      case GD_J3X1: 
      case GD_J3Y1: 
      case GD_J3X2: 
      case GD_J3Y2: 
      case GD_J3T:
         changeJointRow3(hDlg,lParam,wParam);
         break;

      case GD_J4X1: 
      case GD_J4Y1: 
      case GD_J4X2: 
      case GD_J4Y2: 
      case GD_J4T:
         changeJointRow4(hDlg,lParam,wParam);
         break;

      case GD_J5X1: 
      case GD_J5Y1: 
      case GD_J5X2: 
      case GD_J5Y2: 
      case GD_J5T:
         changeJointRow5(hDlg,lParam,wParam);
         break;

      case GD_P1X: 
      case GD_P1Y: 
      case GD_P2X: 
      case GD_P2Y: 
      case GD_P3X: 
      case GD_P3Y:
         changePointRow(hDlg,lParam,wParam);
         break;

      case GD_NUMBOLTS:
         changeNumberOfBolts(hDlg,lParam,wParam);
         break;
  
      case GD_BOLTNUMBER:
         handleBoltNumber(hDlg,lParam,wParam);
         break;
 
      case GD_BOLTX1:
      case GD_BOLTY1:
      case GD_BOLTX2:
      case GD_BOLTY2:
      case GD_BOLTSTIFFNESS:
      case GD_BOLTSTRENGTH:
      case GD_BOLTPRETENSION:
         changeBoltProperties(hDlg,lParam,wParam,boltindex);
         break;

   } /* end switch wmcommand */

  /* FIXME: ensure that this should return true here.
   */
   return TRUE;

} /* close handleWMCommand() */



/* FIXME: This function should easily be able to handle 
 * both the analysis and the geometry dialogs if the 
 * appropriate structure was passed in.  The way to 
 * do this would be to cast all struct to a `vscrollstruct
 * pointer, switch on `i', then cast back to appropriate
 * struct type in the case handler.
 */  
static BOOLEAN
handleVScroll(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   int i,j;

   scrollinfo.cbSize = sizeof(SCROLLINFO);
   scrollinfo.fMask = SIF_POS;

   hScroll = (HWND)lParam;
   i = GetDlgCtrlID(hScroll);
   if(i==GD_JSCROLL) 
   {     
      switch (LOWORD(wParam)) 
      {
         case SB_LINEDOWN: 
            jindex++;
            break;

         case SB_LINEUP: 
            jindex--;  
            break;

         case SB_PAGEDOWN: 
            jindex += 5;  
            break;

         case SB_PAGEUP: 
            jindex -= 5;  
            break;

         case SB_THUMBTRACK: 
            jindex =  HIWORD(wParam); 
            break;
      }

      if(jindex > nj-5) 
         jindex = nj-5;
      if(jindex < 0) 
         jindex=0;
      if(jindex != GetScrollPos(hScroll, SB_CTL))
         SetScrollPos(hScroll, SB_CTL, jindex, TRUE);

      // move joint coords in edit boxes
      if (nj < 5)
         j = nj;
      else
         j = 5;

      for(i=0; i<j; i++) 
      {
         SetDlgItemInt(hDlg, GD_JLAB1+i, jindex+i+1, FALSE);
         gcvt(joints[jindex + i].d1.x, 4, temp);
         SetDlgItemText(hDlg, GD_J1X1+5*i, temp);
         gcvt(joints[jindex + i].d1.y, 4, temp);
         SetDlgItemText(hDlg, GD_J1X1+5*i+1, temp);
         gcvt(joints[jindex + i].d2.x, 4, temp);
         SetDlgItemText(hDlg, GD_J1X1+5*i+2, temp);
         gcvt(joints[jindex + i].d2.y, 4, temp);
         SetDlgItemText(hDlg, GD_J1X1+5*i+3, temp);
         SetDlgItemInt(hDlg, GD_J1X1+5*i+4, joints[jindex + i].type, FALSE);
      }
   } 
   else if(i==GD_PSCROLL) 
   {
      switch (LOWORD(wParam)) 
      {
         case SB_LINEDOWN: 
            pindex++;  
            break;

         case SB_LINEUP: 
            pindex--;  
            break;

         case SB_PAGEDOWN: 
            pindex += 3;  
            break;

         case SB_PAGEUP: 
            pindex -= 3;  
            break;

         case SB_THUMBTRACK: 
            pindex = HIWORD(wParam); 
            break;
      }

      if(pindex > np-3) 
         pindex = np-3;
      if(pindex < 0) 
         pindex=0;
      if(pindex != GetScrollPos(hScroll, SB_CTL))
         SetScrollPos(hScroll, SB_CTL, pindex, TRUE);

      // move point coords in edit boxes
      j = (np < 3)? np: 3;
      for(i=0; i<j; i++) 
      {
         SetDlgItemInt(hDlg, GD_PLAB1+i, pindex+i+1, FALSE);
         gcvt(points[pindex + i].x, 6, temp);
         SetDlgItemText(hDlg, GD_P1X+3*i, temp);
         gcvt(points[pindex + i].y, 6, temp);
         SetDlgItemText(hDlg, GD_P1X+3*i+1, temp);
         temp[0] = ptypes[points[pindex + i].type];
         temp[1] = '\0';
         SetDlgItemText(hDlg, GD_P1X+3*i+2, temp);
      }
   } // end if
   
   return TRUE;

}  /* close handleVScroll() */



static void 
loadDefaults()
{
   int i;

   sprintf(subWinTitle, "Geometry Data: New File");

   nj = 3; nfp = 3; nmp = 0; nlp = 0; nhp = 0;
   joints = (Joint *) malloc(sizeof(Joint) * nj);
   
   for(i=0; i<nj; i++) 
   {
      joints[i].d1.x = 0;
      joints[i].d1.y = 0;
      joints[i].d2.x = 0;
      joints[i].d2.y = 0;
      joints[i].type = 1;
   }

   np = nfp + nmp + nlp + nhp;
   points = (DPoint *) malloc(sizeof(DPoint) * np);

   for(i=0; i<np; i++) 
   {
      points[i].x = 0;
      points[i].y = 0;
      points[i].type = 0;
   }

} /* close loadDefaults() */


static void 
loadFromFile1(HWND hDlg)
{
   int i;

   sprintf(subWinTitle, "Geometry Data: %s", filepath.gfile);
   // read in data from file blocks.out <--- (!?)
   hFile = OpenFile(filepath.gfile, &of, OF_READ);
   fp = fopen(filepath.gfile, "r");
    
  /*  Read in geometry data from file.  */
   fscanf(fp, "%*lf %d %*d %*d %*d %d %d %d %d",
          &nj, &nfp, &nlp, &nmp, &nhp);
  /*  Pseudo-static memory allocation: get the number of 
   *  joints from the file, grab enough memory.
   */
   joints = (Joint *) malloc(sizeof(Joint) * nj);

  /*  Read in all the joints. This technique is prone to 
   *  syntax erros.
   */
   for(i=0; i<nj; i++) 
   {
      fscanf(fp, "%lf %lf %lf %lf %d", &joints[i].d1.x,
             &joints[i].d1.y, &joints[i].d2.x, &joints[i].d2.y,
             &joints[i].type);
   }  /* Close for() reading joints. */

  /* Read in all the points.  These mnemonics are not
   * particulary useful.
   */
   np = nfp + nmp + nlp + nhp;
   points = (DPoint *) malloc(sizeof(DPoint) * np);
   for(i=0; i<nfp; i++) 
   {
      fscanf(fp, "%lf %lf %*lf %*lf", &points[i].x,
             &points[i].y);
      points[i].type = 0;
   }
  /*  This is not too good.  These points are coming in
   * in a particular order.  Very prone to syntax problems.
   */
   for(i=nfp; i<np; i++) 
   {
      fscanf(fp, "%lf %lf", &points[i].x, &points[i].y);
      points[i].type = 1;
      if(i >= nfp+nmp) 
         points[i].type += 1;
      if(i >= nfp+nmp+nlp) 
         points[i].type += 1;
   }
    // hFile = OpenFile(gfile, &of, OF_EXIST);
  _lclose(hFile);

} /* close loadFromFile1() */


static void 
loadFromFile2(HWND hDlg)
{
   int i;

   sprintf(subWinTitle, "Geometry Data: %s", filepath.gfile);
   // read in data from file blocks.out <--- (!?)
   hFile = OpenFile(filepath.gfile, &of, OF_READ);

   //gd = geometryInput(gd, filepath.gfile);
   //geometryInput(gd, filepath.gfile);
   gdata_read_input_file(gd, filepath.gfile);

  /* Now transfer the data to the local structs. */      
   nj = gd->nJoints;
   nfp = gd->nFPoints;
   nlp = gd->nLPoints;
   nmp = gd->nMPoints;
   nhp = gd->nHPoints;

   joints = (Joint *) malloc(sizeof(Joint) * nj);

  /* Transfer joint data   */
   for(i=0; i<nj; i++) 
   {
      joints[i].d1.x = gd->joints[i+1][1];
      joints[i].d1.y = gd->joints[i+1][2]; 
      joints[i].d2.x = gd->joints[i+1][3];
      joints[i].d2.y = gd->joints[i+1][4];
      joints[i].type = (int)gd->joints[i+1][5];
   } 

  /* Read in all the points.  These mnemonics are not
   * particulary useful.
   */
   np = nfp + nmp + nlp + nhp;
   points = (DPoint *) malloc(sizeof(DPoint) * np);

   for (i=0; i<np;i++)
   {
      points[i].type = (int)gd->points[i+1][0];
      points[i].x = gd->points[i+1][1];
      points[i].y = gd->points[i+1][2];
   }
   
   numbolts = gd->nBolts;
   if (numbolts > 0)
      bolts = clone2DMatDoub(gd->rockbolts, gd->rockboltsize1, gd->rockboltsize2);

}  



static BOOLEAN 
setDialogValues(HWND hDlg)
{
   int i,j;

   SetWindowText(hDlg, (LPCTSTR) subWinTitle);
   SetDlgItemInt(hDlg, GD_NJOI, nj, FALSE);
   SetDlgItemInt(hDlg, GD_NFP, nfp, FALSE);
   SetDlgItemInt(hDlg, GD_NMP, nmp, FALSE);
   SetDlgItemInt(hDlg, GD_NLP, nlp, FALSE);
   SetDlgItemInt(hDlg, GD_NHP, nhp, FALSE);

   j = (nj < 5)? nj: 5;
   for(i=0; i<j; i++) 
   {
      gcvt(joints[i].d1.x, 4, temp);
      SetDlgItemText(hDlg, GD_J1X1+5*i, temp);
      gcvt(joints[i].d1.y, 4, temp);
      SetDlgItemText(hDlg, GD_J1X1+5*i+1, temp);
      gcvt(joints[i].d2.x, 4, temp);
      SetDlgItemText(hDlg, GD_J1X1+5*i+2, temp);
      gcvt(joints[i].d2.y, 4, temp);
      SetDlgItemText(hDlg, GD_J1X1+5*i+3, temp);
      SetDlgItemInt(hDlg, GD_J1X1+5*i+4, joints[i].type, FALSE);
   }

   j = (np < 3)? np: 3;
   for(i=0; i<j; i++) 
   {
      gcvt(points[i].x, 6, temp);
      SetDlgItemText(hDlg, GD_P1X+3*i, temp);
      gcvt(points[i].y, 6, temp);
      SetDlgItemText(hDlg, GD_P1X+3*i+1, temp);
      temp[0] = ptypes[points[i].type];
      temp[1] = '\0';
      SetDlgItemText(hDlg, GD_P1X+3*i+2, temp);
   }  /*  End for () loop.  */


   if (numbolts > 0)
      initBoltBoxes(hDlg);
   else 
      enableBoltBoxes(hDlg, FALSE);

  /* Grab some scrollers for the dialog box to deal with 
   * large numbers of points and joints.  
   */
   hScroll = GetDlgItem(hDlg, GD_JSCROLL);
   scrollinfo.fMask = SIF_POS;

   if (nj > 4)
      i = nj-5;
   else 
      i = 0;

   scrollinfo.nPos = i;
   //SetScrollInfo(hScroll, SB_CTL, &si, TRUE);
   SetScrollRange(hScroll, SB_CTL, 0, i, FALSE);
   SetScrollPos(hScroll, SB_CTL, 0, TRUE);

   /*  The point scroller.  */
   hScroll = GetDlgItem(hDlg, GD_PSCROLL);

   if (np > 2)
      i = np-3;
   else 
      i = 0;

   SetScrollRange(hScroll, SB_CTL, 0, i, FALSE);
   SetScrollPos(hScroll, SB_CTL, 0, TRUE);
   
   return TRUE;

}  /* close setDialogValues() */


static BOOLEAN
saveData(HWND hDlg)
{

   int i;
   //FILE * fp;

   
   LPCTSTR szFilter[] = 
      {"Geometry files (*.geo)\0*.geo\0All files (*.*)\0*.*\0\0"};
   fileBrowse(hDlg, &ofn, szFilter, filepath.gpath, filepath.gfile, "geo");

   if( !GetSaveFileName(&ofn) ) 
   {
      strcpy(filepath.gpath, filepath.oldpath);
            //break;  // user pressed cancel
            /* Probably need to return TRUE or FALSE here */
   }
   else 
   { 
      // open file and save data
      // if it exists already, delete it
      if(-1 != OpenFile(filepath.gpath, &of, OF_WRITE) )
         hFile = OpenFile(filepath.gpath, &of, OF_DELETE);
      if(-1 == (hFile = OpenFile(filepath.gpath, &of, OF_CREATE)) ) 
      {
         MessageBox(NULL, "Error: Cannot create file", "ERROR", MB_OK | MB_ICONINFORMATION);
         return 0;
      }


     /* At some point, all these temporary local variables 
      * could be deleted, all the action put in the struct.
      * This would eliminate having to transfer local variable 
      * values to and from the struct.
      */
     /* Now transfer the data to the local structs. */      
      gd->nJoints = nj;
      gd->nFPoints = nfp;
      gd->nLPoints = nlp;
      gd->nMPoints = nmp;
      gd->nHPoints = nhp;
      gd->nBolts = numbolts;

     /* Transfer joint data   */ 
      for(i=0; i<nj; i++) 
      {
         gd->joints[i+1][1] = joints[i].d1.x;
         gd->joints[i+1][2] = joints[i].d1.y; 
         gd->joints[i+1][3] = joints[i].d2.x;
         gd->joints[i+1][4] = joints[i].d2.y;
         gd->joints[i+1][5] = joints[i].type;
      } 

     /* Read in all the points.  These mnemonics are not
      * particulary useful.
      */
      np = nfp + nmp + nlp + nhp;

      for (i=0; i<np;i++)
      {
         gd->points[i+1][0] = points[i].type;
         gd->points[i+1][1] = points[i].x;
         gd->points[i+1][2] = points[i].y;
      }

     /* There will be some memory issues to deal with 
      * here.
      */
      if(numbolts > 0)
         gd->rockbolts = bolts;
         

      //  fp = fopen(filepath.gfile,"w");
     /* Extended format no longer used. */
      //dumpGeometrydata(gd, fp);
      //fclose(fp);
      gd->dumptofile(gd,filepath.gfile);
   }
   
  /* FIXME: verify this return value */
   return TRUE;

}  /* close saveData2() */


static void
changeNumberOfJoints(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   int i,j,k;
   int nOld;

   if (HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         nOld = nj;
         jOld = joints;
         nj = GetDlgItemInt(hDlg, GD_NJOI, NULL, FALSE);
         joints = (Joint *) malloc(sizeof(Joint) * nj);
         if(nj < nOld) 
            nOld = nj;

        /*  These look like cloning loops.  */
         for(i=0; i<nOld; i++) 
         {
            joints[i].d1.x = jOld[i].d1.x;
            joints[i].d1.y = jOld[i].d1.y;
            joints[i].d2.x = jOld[i].d2.x;
            joints[i].d2.y = jOld[i].d2.y;
            joints[i].type = jOld[i].type;
         }
         free(jOld);

         for(i=nOld; i<nj; i++) 
         {
            joints[i].d1.x = 0;
            joints[i].d1.y = 0;
            joints[i].d2.x = 0;
            joints[i].d2.y = 0;
            joints[i].type = 1;
         }

         j = (nj<5)? nj:5;
         jindex = 0;
         for(i=0; i<j; i++) 
         {
            SetDlgItemInt(hDlg, GD_JLAB1+i, i+1, FALSE);
            gcvt(joints[i].d1.x, 4, temp);
            SetDlgItemText(hDlg, GD_J1X1+5*i, temp);
            gcvt(joints[i].d1.y, 4, temp);
            SetDlgItemText(hDlg, GD_J1X1+5*i+1, temp);
            gcvt(joints[i].d2.x, 4, temp);
            SetDlgItemText(hDlg, GD_J1X1+5*i+2, temp);
            gcvt(joints[i].d2.y, 4, temp);
            SetDlgItemText(hDlg, GD_J1X1+5*i+3, temp);
            SetDlgItemInt(hDlg, GD_J1X1+5*i+4, joints[i].type, FALSE);
         }

         temp[0] = '\0';
         for(i=j; i<5; i++) 
         {
            SetDlgItemText(hDlg, GD_JLAB1+i, temp);
            for(k=0; k<5; k++) 
            {
               SetDlgItemText(hDlg, GD_J1X1+5*i+k, temp);
            }
         }

         // reset scroll bar
         hScroll = GetDlgItem(hDlg, GD_JSCROLL);
         i = (nj>4)? nj-5:0;
         SetScrollRange(hScroll, SB_CTL, 0, i, FALSE);
         SetScrollPos(hScroll, SB_CTL, 0, TRUE);
      } // end if
   } // end if

}  /*  close changeNumberOfJoints() */


static void
changeNumberOfPoints(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   int i,j,k;
   int nOld;

   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //if(lParam == EN_KILLFOCUS) {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage((HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage((HWND) lParam, EM_SETMODIFY, FALSE, 0L);

         nOld = np;
         pOld = points;
         nfp = GetDlgItemInt(hDlg, GD_NFP, NULL, FALSE);
         nmp = GetDlgItemInt(hDlg, GD_NMP, NULL, FALSE);
         nlp = GetDlgItemInt(hDlg, GD_NLP, NULL, FALSE);
         nhp = GetDlgItemInt(hDlg, GD_NHP, NULL, FALSE);
         np = nfp + nmp + nlp + nhp;
         points = (DPoint *) malloc(sizeof(DPoint) * np);

         i=0; j=0;
         while (i < nfp && j < nOld && pOld[j].type == 0) 
         {
            points[i].x = pOld[j].x;
            points[i].y = pOld[j].y;
            points[i].type = 0;
            i++; j++;
         }
 
         while (i < nfp) 
         {
            points[i].x = 0; points[i].y = 0;
            points[i].type = 0;
            i++;
         }

         while (i < nfp + nmp && j < nOld && pOld[j].type == 1) 
         {
            points[i].x = pOld[j].x;
            points[i].y = pOld[j].y;
            points[i].type = 1;
            i++; j++;
         }

         while (i < nfp + nmp) 
         {
            points[i].x = 0; points[i].y = 0;
            points[i].type = 1;
            i++;
         }

         while (i < nfp + nmp + nlp && j < nOld && pOld[j].type == 2) 
         {
            points[i].x = pOld[j].x;
            points[i].y = pOld[j].y;
            points[i].type = 2;
            i++; j++;
         }
 
         while (i < nfp + nmp + nlp) 
         {
            points[i].x = 0; points[i].y = 0;
            points[i].type = 2;
            i++;
         }
 
         while (i < nfp + nmp + nlp + nhp && j < nOld && pOld[j].type == 3) 
         {
            points[i].x = pOld[j].x;
            points[i].y = pOld[j].y;
            points[i].type = 3;
            i++; j++;
         }

         while (i < nfp + nmp + nlp + nhp) 
         {
            points[i].x = 0; 
            points[i].y = 0;
            points[i].type = 3;
            i++;
         }

         free(pOld);


         j = (np<3)? np:3;
         pindex = 0;
         for(i=0; i<j; i++) 
         {
            SetDlgItemInt(hDlg, GD_PLAB1+i, pindex+i+1, FALSE);
            gcvt(points[i].x, 4, temp);
            SetDlgItemText(hDlg, GD_P1X+3*i, temp);
            gcvt(points[i].y, 4, temp);
            SetDlgItemText(hDlg, GD_P1X+3*i+1, temp);
            temp[0] = ptypes[points[i].type];
            temp[1] = '\0';
            SetDlgItemText(hDlg, GD_P1X+3*i+2, temp);
         }

         temp[0] = '\0';
         for(i=j; i<3; i++) 
         {
            SetDlgItemText(hDlg, GD_PLAB1+i, temp);
            for(k=0; k<3; j++) 
            {
               SetDlgItemText(hDlg, GD_P1X+3*i+k, temp);
            }
         }
 
         // reset scroll bar
         hScroll = GetDlgItem(hDlg, GD_PSCROLL);
         i = (np>2)? np-3:0;
         SetScrollRange(hScroll, SB_CTL, 0, i, FALSE);
         SetScrollPos(hScroll, SB_CTL, 0, TRUE);
      } // end if

   } // end if

}  /*  close changeNumberOfPoints() */


static void 
changeJointRow1(HWND hDlg,LPARAM lParam,WPARAM wParam)
{
   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L))
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         GetDlgItemText(hDlg, GD_J1X1, temp, 20);
         joints[jindex + 0].d1.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J1Y1, temp, 20);
         joints[jindex + 0].d1.y = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J1X2, temp, 20);
         joints[jindex + 0].d2.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J1Y2, temp, 20);
         joints[jindex + 0].d2.y = strtod(temp, NULL);
         joints[jindex + 0].type = GetDlgItemInt(hDlg, GD_J1T, NULL, FALSE);
      } // end if

   } // end if

} /* close changeJointRow1() */


static void 
changeJointRow2(HWND hDlg,LPARAM lParam,WPARAM wParam)
{
   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //if(lParam == EN_KILLFOCUS) {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         GetDlgItemText(hDlg, GD_J2X1, temp, 20);
         joints[jindex + 1].d1.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J2Y1, temp, 20);
         joints[jindex + 1].d1.y = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J2X2, temp, 20);
         joints[jindex + 1].d2.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J2Y2, temp, 20);
         joints[jindex + 1].d2.y = strtod(temp, NULL);
         joints[jindex + 1].type = GetDlgItemInt(hDlg, GD_J2T, NULL, FALSE);
      } // end if

   } // end if

} /* close changeJointRow2() */


static void 
changeJointRow3(HWND hDlg,LPARAM lParam,WPARAM wParam)
{
   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         GetDlgItemText(hDlg, GD_J3X1, temp, 20);
         joints[jindex + 2].d1.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J3Y1, temp, 20);
         joints[jindex + 2].d1.y = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J3X2, temp, 20);
         joints[jindex + 2].d2.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J3Y2, temp, 20);
         joints[jindex + 2].d2.y = strtod(temp, NULL);
         joints[jindex + 2].type = GetDlgItemInt(hDlg, GD_J3T, NULL, FALSE);
      } // end if

   } // end if

} /* close changeJointRow3() */


static void 
changeJointRow4(HWND hDlg,LPARAM lParam,WPARAM wParam)
{
   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage((HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         GetDlgItemText(hDlg, GD_J4X1, temp, 20);
         joints[jindex + 3].d1.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J4Y1, temp, 20);
         joints[jindex + 3].d1.y = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J4X2, temp, 20);
         joints[jindex + 3].d2.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J4Y2, temp, 20);
         joints[jindex + 3].d2.y = strtod(temp, NULL);
         joints[jindex + 3].type = GetDlgItemInt(hDlg, GD_J4T, NULL, FALSE);
      } // end if

   } // end if

} /* close changeJointRow4() */


static void 
changeJointRow5(HWND hDlg,LPARAM lParam,WPARAM wParam)
{
   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L))
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         GetDlgItemText(hDlg, GD_J5X1, temp, 20);
         joints[jindex + 4].d1.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J5Y1, temp, 20);
         joints[jindex + 4].d1.y = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J5X2, temp, 20);
         joints[jindex + 4].d2.x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_J5Y2, temp, 20);
         joints[jindex + 4].d2.y = strtod(temp, NULL);
         joints[jindex + 4].type = GetDlgItemInt(hDlg, GD_J5T, NULL, FALSE);
      } // end if

   } // end if

} /* close changeJointRow5() */


static void
changePointRow(HWND hDlg,LPARAM lParam,WPARAM wParam)
{
   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L))
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         GetDlgItemText(hDlg, GD_P1X, temp, 20);
         points[pindex + 0].x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_P1Y, temp, 20);
         points[pindex + 0].y = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_P2X, temp, 20);
         points[pindex + 1].x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_P2Y, temp, 20);
         points[pindex + 1].y = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_P3X, temp, 20);
         points[pindex + 2].x = strtod(temp, NULL);
         GetDlgItemText(hDlg, GD_P3Y, temp, 20);
         points[pindex + 2].y = strtod(temp, NULL);
        /* change this to the loop assuming the 
         * text boxes for the points are in 
         * correct order
         */
        /*
         for (i=0;i<3;i++)
         {
            GetDlgItemText(hDlg, GD_P1X+2*i, temp, 20);
            points[pindex + i].x = strtod(temp, NULL);
            GetDlgItemText(hDlg, GD_P1Y+2*i+1, temp, 20);
            points[pindex + i].y = strtod(temp, NULL);
         }
         */
      } // end if

   } // end if

} /* close changePointRow() */


/* This edit box is read-only for now due to 
 * memory handling problems and over all bad
 * design.  This is probably a throw-away
 * function.
 */
static void
setNumberOfBolts(HWND hDlg)
{
   SetDlgItemInt(hDlg, GD_NUMBOLTS, numbolts, FALSE);
} /* close setNumberOfBolts() */


/* This doesn't really work very well.  For one, there
 * is a memory problem associated with copying the
 * arrays that makes it segfault when more newer than
 * older bolts happen.
 */
static void 
changeNumberOfBolts(HWND hDlg,LPARAM lParam,WPARAM wParam)
{
   int numOld;
   int i,j;
   //int tmp;

   if (HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);

         numOld = numbolts;
         oldbolts = bolts;

         numbolts = GetDlgItemInt(hDlg, GD_NUMBOLTS, NULL, FALSE);
         bolts = DoubMat2DGetMem(numbolts, 14);

        /* The way this is handled is to have the 
         * bolts as an ordered set.  Changing the 
         * number of bolts results in either truncating
         * the list of bolts if the new number is less
         * than the old number, or adding new bolts to
         * the end of the list if the more bolts are 
         * added.  This is not a particularly flexible 
         * implementation if one wants to say delete 
         * one or more bolts in the middle of the list.
         */
         if(numbolts < numOld) 
            numOld = numbolts;

        /* Clone the first part of the bolt array */
         bolts = clone2DMatDoub(oldbolts,numbolts,14);
         free2DMat(oldbolts,numOld);
 
        /* If the number of new bolts is greater than the 
         * number of old bolts, initialize to zero, 
         * else skip.
         */
         for(i=numOld; i<numbolts; i++) 
            for(j=0;j<14;j++)
               bolts[i][j] = 0.0;

         boltindex = 0;
         
         for (i=1;i<=4;i++)
         {
            gcvt(bolts[0][i], 4, temp);
            SetDlgItemText(hDlg, GD_BOLTX1+i-1, temp);
         }

        /* Probably sets labels */
        /*
         temp[0] = '\0';
         for(i=j; i<5; i++) 
         {
            SetDlgItemText(hDlg, GD_JLAB1+i, temp);
            for(k=0; k<5; k++) 
            {
               SetDlgItemText(hDlg, GD_J1X1+5*i+k, temp);
            }
         }
         */

      } // end if
   } // end if
} /* close changeNumberOfBolts() */



static void 
changeBoltProperties(HWND hDlg,LPARAM lParam,WPARAM wParam, int index)
{
   int i;

   if (HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);

         for (i=7;i<10;i++)
         {
           /* Use BOLTX1 because BOLTSTIFFNESS is the 
            * 7th text box up, and we can use i to 
            * index the bolt array.
            */
            GetDlgItemText(hDlg, GD_BOLTX1+i, temp, 20);
            bolts[index][i] = strtod(temp, NULL);
         }

      }

   }


} /* close changeBoltProperties() */



static void 
enableBoltBoxes(HWND hDlg, BOOLEAN flag)
{
   int i;

   for (i=0;i<7;i++)
   {
      hEditCtrl = GetDlgItem(hDlg, GD_BOLTX1+i);
      EnableWindow(hEditCtrl, flag);
   }
  /* Since BOLTNUMBER is not adjacently numbered, it
   * has to be handled separately.
   */
   hEditCtrl = GetDlgItem(hDlg, GD_BOLTNUMBER);
   EnableWindow(hEditCtrl, flag);

}  /* close enableBoltBoxes() */

static void
initBoltBoxes(HWND hDlg)
{
   int i;

  /* Now set the combo box */
   for (i=0;i<numbolts;i++)
      SendDlgItemMessage(hDlg, GD_BOLTNUMBER, CB_ADDSTRING, 0, (LPARAM)((LPCSTR) numlist[i]));

  /* Set the combo box to point to bolt 1 */
   SendDlgItemMessage(hDlg, GD_BOLTNUMBER, CB_SETCURSEL, 0, 0);

  /* Display the first (number 0) bolt in the 
   * list of bolts.
   */
   setBoltBoxes(hDlg, 0);

  /* Set the number of bolts into a read-only box. 
   */
   SetDlgItemInt(hDlg, GD_NUMBOLTS, numbolts, FALSE);

}  /* close initBoltBoxes() */

static void
setBoltBoxes(HWND hDlg, int index)
{
   int j;

   for (j=1;j<5;j++)
   {
      gcvt(bolts[index][j], 6, temp);
      SetDlgItemText(hDlg, GD_BOLTX1+j-1, temp);
   }

  /* The block numbers are stored in j = 5,6,
   * so we have to skip those numbers.  Since 
   * win32 controls expect linear progression,
   * its easier to do an extra loop rather than
   * add yet one more layer of crufty kludge
   */
   for (j=7;j<10;j++)
   {
      gcvt(bolts[index][j], 6, temp);
      SetDlgItemText(hDlg, GD_BOLTX1+j-3, temp);
   }
  
}  /* close initBoltBoxes() */


static void 
handleBoltNumber(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   boltindex = (int)SendDlgItemMessage(hDlg, GD_BOLTNUMBER, CB_GETCURSEL, 0,0L);
   setBoltBoxes(hDlg, boltindex);

}  /* close handleBoltNumber() */