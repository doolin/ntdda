/* 
 * analysisdialog.c
 * 
 * Handles the result of message passing from the 
 * analysis dialog box.
 * $Author: doolin $
 * $Date: 2001/11/02 13:38:42 $
 * $Source: /cvsroot/dda/ntdda/src/win32gui/analysisdialog.c,v $
 * $Revision: 1.2 $
 */

#include "analysisdlg.h"
#include "ddamemory.h"
#include "interface.h"

 
/* Static variables declared outside of function calls
 * are file specific, that is, these following variables
 * are local to this file, but global to the functions
 * in this file.  Many of these can undoubtedly be moved 
 * back into a local variable status at a later date.
 */

static int dynam = 1, nts = 10, njmat = 1, nbmat = 1;
/* FIXME: move jindex to a function handling joint boxes. */
static int jindex = 0;
static int bindex = 0;
static int loadindex = 0;
/* scrollpos = vscroll position
 */
static int lpscrollpos = 0;  
static int njmatOld = 0, nbmatOld = 0;
static JointMat *jmat, *jmatOld = NULL;
static BlockMat *bmat, *bmatOld = NULL;
static double maxdisp = .01;
static char temp[20];
static char *numlist[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", \
        "12", "13", "14", "15", "16", "17", "18", "19", "20"};
static char testText[21];
/* Might be able to get rid of this one. */
static char subWinTitle[40];
HWND hLPScroller;
SCROLLINFO scrollinfo;

static int gravity;
static int rotation;
static int planestrain;
static int autotimestep = 1;
static int autopenalty = 1;
static int numtdeps;
/* The load points are an array of structs */
static LOADPOINT * lpoints;
static Analysisdata * ad;

/* Indicates the interval between saving time steps.
 * Default is to save every 5th time step.  This
 * can probably be moved to a local or a struct
 * member.  Not implemented right now because time
 * stepping problems in the analysis code.  FIXME:
 * reimplement tsSaveInterval through using the 
 * struct.
 */
int tsSaveInterval;

/* Helper functions to clean up the main message loop
 * handled by this dialog box.
 */
static void loadFileData(HWND, HFILE *, OFSTRUCT *);
static void loadDefaults();
static void setDialogValues(HWND hDlg, LPARAM, WPARAM);
static void saveData();
static void setLoadCells(HWND, int, int);
static void handleScrollBar(HWND, LPARAM, WPARAM, int);
static BOOLEAN handleWMCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
static void enableLoadBoxes(HWND hDlg, BOOLEAN flag);
static void handleJointPlus(HWND hDlg);
static void handleJointMinus(HWND hDlg);

extern InterFace * iface;


/* Externs from winmain.c  */
extern FILEPATHS filepath;


/* This is now passed in from the dialog box call
 * as an lparam or something and is cast out of the 
 * the pointer variable.  See the init function.
 */
//static Geometrydata * geomdata;

static void handleInitAnalysisDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);


BOOL CALLBACK AnalDlgProc (HWND hDlg, 
         UINT iMessage, 
         WPARAM wParam, 
         LPARAM lParam)
{

   switch (iMessage)
   {
     case WM_INITDIALOG :
        handleInitAnalysisDialog(hDlg,wParam,lParam);
        return TRUE;

     case WM_VSCROLL:
        handleScrollBar(hDlg, lParam, wParam, loadindex);
        return TRUE;

     case WM_COMMAND:
        return(handleWMCommand(hDlg, wParam, lParam));

     default:
        return FALSE;

   } /* end switch */

   return TRUE ;

}   // end AnalDlgProc



static void 
handleInitAnalysisDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
   OFSTRUCT of;
   HFILE hFile;

  /* Get rid of this by refusing to read in anything but the 
   * ddaml format.
   */
   //geomdata = (Geometrydata *)lParam;

   if(filepath.afile[0] != '\0') 
   {
      iface->setdisplay((unsigned int)hDlg);
      loadFileData(hDlg, &hFile, &of);
   } 
   else 
   {
      loadDefaults();
   }   /* end if INITDIALOG  */

   // put values in dialog box
   setDialogValues(hDlg, lParam, wParam);


}  /* close handleInitAnalysisDialog() */


static BOOLEAN
handleADSave(HWND hDlg, WPARAM wParam, LPARAM lParam)
{

   OPENFILENAME ofn;
   OFSTRUCT of;
   HFILE hFile;


   LPCTSTR szFilter[] = 
                {"Analysis files (*.ana)\0*.ana\0All files (*.*)\0*.*\0\0"};
   fileBrowse(hDlg, &ofn, szFilter, filepath.oldpath, filepath.apath, "ana");
   nts = GetDlgItemInt(hDlg, AD_NTS, NULL, FALSE);
   tsSaveInterval = GetDlgItemInt(hDlg, AD_SAVEINT, NULL, FALSE);
   GetDlgItemText(hDlg, AD_MAXDISP, temp, 20);
   maxdisp = strtod(temp, NULL);

   if( !GetSaveFileName(&ofn) ) 
   {
      strcpy(filepath.apath, filepath.oldpath);
      return FALSE;  // user pressed cancel
   } 
   else 
   { // open file and save data
            // if it exists already, delete it
      if(-1 != OpenFile(filepath.apath, &of, OF_WRITE) )
         hFile = OpenFile(filepath.apath, &of, OF_DELETE);

      if(-1 == (hFile = OpenFile(filepath.apath, &of, OF_CREATE)) ) {
         MessageBox(NULL, "Error: Cannot create file", "ERROR", MB_OK | MB_ICONINFORMATION);
         return FALSE;
         }
             
      saveData();
     _lclose(hFile);
      EndDialog (hDlg, 1);
      return TRUE;
   } // end if

}  /* close handleADSave() */


static BOOLEAN
handleWMCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{

   int nlp;
   int i,j;

   switch (LOWORD(wParam))
   {
     /* What follows can eventually be moved to  
      * a handler function.
      */
      case AD_SAVE:
         handleADSave(hDlg,wParam,lParam);
         break;

      case IDCANCEL:
         tsSaveInterval = GetDlgItemInt(hDlg, AD_SAVEINT, NULL, FALSE);
         if(jmatOld) 
            free(jmatOld);
        /* FIXME: Seg fault here if you add and subtract a joint using the +/- 
         * buttons.  This points out that memory management in the
         * dialog boxes is probably not too good.
         */
        /*
         if(jmat)
            free(jmat);
         */
         if(bmatOld) 
            free(bmatOld);
         free(bmat);

        /* FIXME: free the analysis struct also, if 
         * it is a new one.  Else, don't free it.
         */
         EndDialog (hDlg, 0);
         return TRUE;

   case AD_STAT:
      dynam = 0;
      break;

   case AD_DYNAM:
      dynam = 1;
      break;

   case AD_PSTRESS:
      planestrain = 0;
      break;

   case AD_PSTRAIN:
      planestrain = 1;
      break;

   case AD_GRAVITY:
      gravity = 1;
      break;
  
   case AD_ROTATION:
      rotation = 1;
      break;

   case AD_JPLUS:
      SendMessage(hDlg, WM_COMMAND, AD_JMAT, 0L);
      handleJointPlus(hDlg);
      break;

   case AD_JMINUS:
      SendMessage(hDlg, WM_COMMAND, AD_JMAT, 0L);
      handleJointMinus(hDlg);
      break;

   case AD_JMAT:
      i = (int) SendDlgItemMessage(hDlg, AD_JMAT, CB_GETCURSEL, 0, 0L);
      gcvt(jmat[i].fric, 4, temp);
      SetDlgItemText(hDlg, AD_FRIC, temp);
      gcvt(jmat[i].coh, 4, temp);
      SetDlgItemText(hDlg, AD_COH, temp);
      gcvt(jmat[i].tens, 4, temp);
      SetDlgItemText(hDlg, AD_TENS, temp);
      jindex = i;
      break;

   case AD_TIMEDEPS:
      loadindex = (int) SendDlgItemMessage(hDlg, AD_TIMEDEPS, CB_GETCURSEL, 0, 0L);
     /* Set the scroll bar position */ 
     /* FIXME: Combine with other code to set 
      * scroll bar and make a function out of it.
      * FIXME: Clicking on this box when adding a new
      * analysis produces a segfault.  Need to check
      * for null lpoints.
      */
     /* Kludgy fix.  Works for now.
      */
      if (lpoints == NULL)
         nlp = 0;
      else
         nlp = lpoints[loadindex].loadpointsize1;

      if (nlp > 3)
         i = nlp-4;
      else 
         i = 0;

      SetScrollRange(hLPScroller, SB_CTL, 0, i, FALSE);
      SetScrollPos(hLPScroller, SB_CTL, 0, TRUE);
      /* May need the following: */
      if (nlp > 0)
         setLoadCells(hDlg, loadindex, 0);
      break;

   case AD_BPLUS:
      SendMessage(hDlg, WM_COMMAND, AD_BMAT, 0L);
      if(nbmat < 20) 
      {
         if(bmatOld && bmatOld != bmat) 
            free(bmatOld);
         bmatOld = bmat;
         nbmatOld = nbmat;
         nbmat++;
         bmat = (BlockMat *) malloc(sizeof(BlockMat) * nbmat);
         for(i = 0; i<nbmatOld; i++) 
         {
            bmat[i].dens = bmatOld[i].dens;
            bmat[i].wt = bmatOld[i].wt;
            bmat[i].ymod = bmatOld[i].ymod;
            bmat[i].pois = bmatOld[i].pois;

            for(j=0; j<3; j++) 
               bmat[i].iss[j] = bmatOld[i].iss[j];
            for(j=0; j<3; j++) 
               bmat[i].ist[j] = bmatOld[i].ist[j];
            for(j=0; j<3; j++) 
               bmat[i].ivel[j] = bmatOld[i].ivel[j];
         }

         for(i = nbmatOld; i<nbmat; i++) 
         {
            bmat[i].dens = 2.5;
            bmat[i].wt = 25;
            bmat[i].ymod = 100000;
            bmat[i].pois = .49;
  
            for(j=0; j<3; j++) 
               bmat[i].iss[j] = 0;
            for(j=0; j<3; j++) 
               bmat[i].ist[j] = 0;
            for(j=0; j<3; j++) 
               bmat[i].ivel[j] = 0;
         }

         SetDlgItemInt(hDlg, AD_NBMAT, nbmat, FALSE);
         SendDlgItemMessage(hDlg, AD_BMAT, CB_RESETCONTENT, 0, 0);
         for(i=0; i<nbmat; i++) 
         {
            SendDlgItemMessage(hDlg, AD_BMAT, CB_ADDSTRING, 0, (LPARAM) ((LPCSTR) numlist[i]));
         }
         SendDlgItemMessage(hDlg, AD_BMAT, CB_SETCURSEL, 0, 0);
         gcvt(bmat[0].dens, 4, temp);
         SetDlgItemText(hDlg, AD_DENS, temp);
         gcvt(bmat[0].wt, 4, temp);
         SetDlgItemText(hDlg, AD_WT, temp);
         gcvt(bmat[0].ymod, 4, temp);
         SetDlgItemText(hDlg, AD_YMOD, temp);
         gcvt(bmat[0].pois, 4, temp);
         SetDlgItemText(hDlg, AD_POIS, temp);

         for(i=0; i<3; i++) 
         {
            gcvt(bmat[0].iss[i], 4, temp);
            SetDlgItemText(hDlg, AD_ISSX+i, temp);
         }

         for(i=0; i<3; i++) 
         {
            gcvt(bmat[0].ist[i], 4, temp);
            SetDlgItemText(hDlg, AD_ISTX+i, temp);
         }

         for(i=0; i<3; i++) 
         {
            gcvt(bmat[0].ivel[i], 4, temp);
            SetDlgItemText(hDlg, AD_IVELX+i, temp);
         }
 
         bindex = 0;
      }  //end if
      break;

   case AD_BMINUS:
      SendMessage(hDlg, WM_COMMAND, AD_BMAT, 0L);
      if(nbmat > 1) 
      {
         if(bmatOld && bmatOld != bmat) 
            free(bmatOld);

         bmatOld = bmat;
         nbmatOld = nbmat;
         nbmat--;
         /* This (itoa) probably needs to be replaced with 
          * an sprintf.
          */
         itoa(nbmat, temp, 10);
         SetDlgItemText(hDlg, AD_NBMAT, temp);
         SendDlgItemMessage(hDlg, AD_BMAT, CB_RESETCONTENT, 0, 0);

         for(i=0; i<nbmat; i++) 
         {
            SendDlgItemMessage(hDlg, AD_BMAT, CB_ADDSTRING, 0, (LPARAM) ((LPCSTR) numlist[i]));
         }

         SendDlgItemMessage(hDlg, AD_BMAT, CB_SETCURSEL, 0, 0);
         gcvt(bmat[0].dens, 4, temp);
         SetDlgItemText(hDlg, AD_DENS, temp);
         gcvt(bmat[0].wt, 4, temp);
         SetDlgItemText(hDlg, AD_WT, temp);
         gcvt(bmat[0].ymod, 4, temp);
         SetDlgItemText(hDlg, AD_YMOD, temp);
         gcvt(bmat[0].pois, 4, temp);
         SetDlgItemText(hDlg, AD_POIS, temp);

         for(i=0; i<3; i++) 
         {
            gcvt(bmat[0].iss[i], 4, temp);
            SetDlgItemText(hDlg, AD_ISSX+i, temp);
         }

         for(i=0; i<3; i++) 
         {
            gcvt(bmat[0].ist[i], 4, temp);
            SetDlgItemText(hDlg, AD_ISTX+i, temp);
         }

         for(i=0; i<3; i++) 
         {
            gcvt(bmat[0].ivel[i], 4, temp);
            SetDlgItemText(hDlg, AD_IVELX+i, temp);
         }
         bindex = 0;
      }  // end if
      break;

   case AD_BMAT:
      i = (int) SendDlgItemMessage(hDlg, AD_BMAT, CB_GETCURSEL, 0, 0L);
      gcvt(bmat[i].dens, 4, temp);
      SetDlgItemText(hDlg, AD_DENS, temp);
      gcvt(bmat[i].wt, 4, temp);
      SetDlgItemText(hDlg, AD_WT, temp);
      gcvt(bmat[i].ymod, 4, temp);
      SetDlgItemText(hDlg, AD_YMOD, temp);
      gcvt(bmat[i].pois, 4, temp);
      SetDlgItemText(hDlg, AD_POIS, temp);
      for(i=0; i<3; i++) 
      {
         gcvt(bmat[i].iss[i], 4, temp);
         SetDlgItemText(hDlg, AD_ISSX+i, temp);
      }
 
      for(i=0; i<3; i++) 
      {
         gcvt(bmat[i].ist[i], 4, temp);
         SetDlgItemText(hDlg, AD_ISTX+i, temp);
      }

      for(i=0; i<3; i++) 
      {
         gcvt(bmat[i].ivel[i], 4, temp);
         SetDlgItemText(hDlg, AD_IVELX+i, temp);
      }

      bindex = i;
      break;

  /* Friction, cohesion and tensile strength edit controls. */
   case AD_FRIC: 
   case AD_COH: 
   case AD_TENS:
   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      // MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", NULL);
      if (SendMessage((HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage((HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         GetDlgItemText(hDlg, AD_FRIC, temp, 20);
         jmat[jindex].fric = strtod(temp, NULL);
         GetDlgItemText(hDlg, AD_COH, temp, 20);
         jmat[jindex].coh = strtod(temp, NULL);
         GetDlgItemText(hDlg, AD_TENS, temp, 20);
         jmat[jindex].tens = strtod(temp, NULL);
      } // end if
   } // end if
   break;

  /*  Mass density, weight, Poisson's ratio, etc.  saving 
   * worked on June 26 1998.  
   */
   case AD_DENS: 
   case AD_WT: 
   case AD_YMOD: 
   case AD_POIS:
   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      // MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", NULL);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
         GetDlgItemText(hDlg, AD_DENS, temp, 20);
         bmat[bindex].dens = strtod(temp, NULL); 
         GetDlgItemText(hDlg, AD_WT, temp, 20);
         bmat[bindex].wt = strtod(temp, NULL);
         GetDlgItemText(hDlg, AD_YMOD, temp, 20);
         bmat[bindex].ymod = strtod(temp, NULL);
         GetDlgItemText(hDlg, AD_POIS, temp, 20);
         bmat[bindex].pois = strtod(temp, NULL);
      } // end if
   } // end if
   break;

  /*  Load the edit controls with initial values of
   * stress, strain and initial velocity.  These worked 
   * test file as of 25 June, 1998.  However, traversing 
   * over to the number of blocks control changes the values
   * in these controls.  
   */
   case AD_ISSX:  
   case AD_ISSY:  
   case AD_ISSXY:
   case AD_ISTX: 
   case AD_ISTY:
   case AD_ISTXY:
   case AD_IVELX: 
   case AD_IVELY: 
   case AD_IVELR:
      if(HIWORD(wParam) == EN_KILLFOCUS) 
      {
         // MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", NULL);
         if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
         {
            SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);

            for(i=0; i<3; i++) 
            {
               GetDlgItemText(hDlg, AD_ISSX+i, temp, 20);
               bmat[bindex].iss[i] = strtod(temp, NULL);
            }
 
            for(i=0; i<3; i++) 
            {
               GetDlgItemText(hDlg, AD_ISTX+i, temp, 20);
               bmat[bindex].ist[i] = strtod(temp, NULL);
            }

            for(i=0; i<3; i++) 
            {
               GetDlgItemText(hDlg, AD_IVELX+i, temp, 20);
               bmat[bindex].ivel[i] = strtod(temp, NULL);
            }
         } // end if
      } // end if
      break;

  /* Handle the load points.
   */
   case AD_STARTTIME1:  
   case AD_FORCEX1:  
   case AD_FORCEY1:
   case AD_STARTTIME2: 
   case AD_FORCEX2:
   case AD_FORCEY2:
   case AD_STARTTIME3: 
   case AD_FORCEX3: 
   case AD_FORCEY3:
   case AD_STARTTIME4:
   case AD_FORCEX4:
   case AD_FORCEY4:
      if(HIWORD(wParam) == EN_KILLFOCUS) 
      {
         // MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", NULL);
         if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L)) 
         {
            int k = 0;
            SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
/* FIXME: I am sure this is necessary to handle less than 
 * 4 loadpoint time values.
 */               
/*
if (lpoints[index].loadpointsize1 < 4)
   j = lpoints[index].loadpointsize1;
else 
   j = 4;
*/
//int nCells; /* might be less values than cells. */
            for(i=0; i<4; i++) 
            {
               for (j=0; j<3; j++)
               {  
                  GetDlgItemText(hDlg, AD_STARTTIME1+k, temp, 20);
                  lpoints[loadindex].vals[lpscrollpos+i][j] = strtod(temp, NULL);
                  k++;
               }
            }
         } // end if
      } // end if
      break;

   } /* end switch (wparam)  */

  /* FIXME: verify that this return isn't lost */
   return TRUE;

} /* close handleWMCommand() */


/* What follows is a set of helper functions that allow
 * much cleaner message handling in the main body of the 
 * analysis dialog code.  All of the static variables 
 * declared in this file should be initialized from 
 * this function.  No members of the analysis struct
 * (ad) should be used outside of this function and the 
 * save function.
 */
static void 
loadFileData(HWND hDlg, HFILE * hFile, OFSTRUCT * of)
{
   int i;


  /* The following block of code is what actually reads the file 
   * information into the analysis dialog box.  This will eventually
   * be replaced with a function that will translate the analysis
   * data structure arrays into the structs data needed for the file
   * dialog.  Good project for later.
   */
   //sprintf(subWinTitle, "Analysis Data: %s", filepath.afile);
   // open afile and read in data
  //*hFile = OpenFile(filepath.afile, of, OF_READ);

  /* FIXME: Get rid of dependence of geometry input files. 
   * This can only be done by either eliminating everything 
   * but ddaml input, or by trapping other formats and 
   * refusing to proceed.
   */
   //ad = analysisInput(filepath.afile, geomdata);
   ad = XMLparseDDA_Analysis_File(filepath.afile);
   assert (ad != NULL);

  /* Kludge: if not ddaml, then inform the user and kill
   * the dialog box...
   */

   if(ad->fileformat != ddaml)
   {
      MessageBox(hDlg,"Analysis interface only supports\n the Berkeley DDAML input format.","Error",MB_OK);
      //SendMessage(hDlg,WM_QUITDIALOG,0,0);
   }


   ///fp = fopen(filepath.afile, "r");

  /* Might want to reimplement time step saving interval
   * as well.
   */

   dynam = ad->analysistype;
   nts = ad->nTimeSteps;
   nbmat = ad->nBlockMats;
   njmat = ad->nJointMats;
   //maxdisp = ad->g2;
   maxdisp = ad->maxdisplacement;

   /* FIXME: Add max time step here */
   // maxtimestep = ad->maxtimestep;

   rotation = ad->rotationflag;
   gravity = ad->gravityflag;
   planestrain = ad->planestrainflag;
   autotimestep = ad->autotimestepflag;
   autopenalty = ad->autopenaltyflag;

  /* Get the materials and initial state of each 
   * block material. 
   */
   bmat = (BlockMat *) malloc(sizeof(BlockMat) * nbmat);


   for(i=0; i<nbmat; i++) 
   {
      bmat[i].dens = ad->materialProps[i+1][0];
      bmat[i].wt = ad->materialProps[i+1][1];
      bmat[i].ymod = ad->materialProps[i+1][2];
      bmat[i].pois = ad->materialProps[i+1][3];
      bmat[i].iss[0] = ad->materialProps[i+1][4];
      bmat[i].iss[1] = ad->materialProps[i+1][5];
      bmat[i].iss[2] = ad->materialProps[i+1][6];
      bmat[i].ist[0] = ad->materialProps[i+1][7];
      bmat[i].ist[1] = ad->materialProps[i+1][8];
      bmat[i].ist[2] = ad->materialProps[i+1][9];
      bmat[i].ivel[0] = ad->materialProps[i+1][10];
      bmat[i].ivel[1] = ad->materialProps[i+1][11];
      bmat[i].ivel[2] = ad->materialProps[i+1][12];
   }

  /* Get all of the joint materials for each type.  */
   jmat = (JointMat *) malloc(sizeof(JointMat) * njmat);

   for(i=0; i<njmat; i++) 
   {
      jmat[i].fric = ad->phiCohesion[i+1][0];
      jmat[i].coh = ad->phiCohesion[i+1][1];
      jmat[i].tens = ad->phiCohesion[i+1][2];
   }
      
  /* This is a really bad business here. 
   * FIXME: Time dependent points code needs to 
   * be rewritten very badly.  This probably
   * will not happen until the entire point
   * handling code gets rewritten.
   */
   numtdeps = ad->nTDPoints; //ad->nLPoints;
   lpoints = cloneLoadPoints(ad);


    // hFile = OpenFile(afile, &of, OF_EXIST);
    //      _lclose(*hFile);
           
}  /* close loadFileData2() */


/* FIXME: Initialize all of the static analysis variables
 * declared in this file, then initialize the analysis 
 * struct with these variables in the save routine, right
 * before the call to dumpAnalysisData(). 
 */
static void 
loadDefaults()
{
   int i, j;

  /* Grab an instance of an analysis struct.
   * we will need it for writing out the data.
   */

   ad = adata_new();

   ad->nBlockMats = nbmat = 1;
   ad->nJointMats = njmat = 1;

  /* use default values */
   sprintf(subWinTitle, "Analysis Data: New File");
   jmat = (JointMat *) malloc(sizeof(JointMat) * njmat);
   for(i = 0; i<njmat; i++) 
   {
      jmat[i].fric = 0;
      jmat[i].coh = 0;
      jmat[i].tens = 0;
   }
    
   bmat = (BlockMat *) malloc(sizeof(BlockMat) * nbmat);
   for (i = 0; i<nbmat; i++) 
   {
      bmat[i].dens = 2.5;
      bmat[i].wt = 25;
      bmat[i].ymod = 1000000;
      bmat[i].pois = .49;

      for(j=0; j<3; j++) 
         bmat[i].iss[j] = 0;

      for(j=0; j<3; j++) 
         bmat[i].ist[j] = 0;

      for(j=0; j<3; j++) 
         bmat[i].ivel[j] = 0;
   }

  /* Kludge: make numtdeps depend on 
   * having a valid geometry file loaded.
   */
   //numtdeps = geomdata->nFPoints+geomdata->nLPoints;
  /* FIXME: disable the time deps boxes using
   * a function call
   */

}  /* close loadDefaults() */



/* This is the main deal where all the GUI stuff
 * gets the display set.
 */
static void
setDialogValues(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   int i;
   int nlp;  /* number of load points */

  /* Analysis flags */
   SetWindowText(hDlg, (LPSTR) subWinTitle);
   CheckRadioButton(hDlg, AD_STAT, AD_DYNAM, AD_STAT + dynam);
   CheckRadioButton(hDlg, AD_PSTRESS, AD_PSTRAIN, AD_PSTRESS + planestrain);

   if (gravity) 
      CheckDlgButton(hDlg, AD_GRAVITY, MF_CHECKED);
   else 
      CheckDlgButton(hDlg, AD_GRAVITY, MF_UNCHECKED);

   if (rotation) 
      CheckDlgButton(hDlg, AD_ROTATION, MF_CHECKED);
   else 
      CheckDlgButton(hDlg, AD_ROTATION, MF_UNCHECKED);

  /* Analysis parameters */
   SetDlgItemInt(hDlg, AD_NTS, nts, FALSE);
   tsSaveInterval = (int) ((nts-1)/20 + 1);
   SetDlgItemInt(hDlg, AD_SAVEINT, tsSaveInterval, FALSE);
   SetDlgItemInt(hDlg, AD_NJMAT, njmat, FALSE);
   SetDlgItemInt(hDlg, AD_NBMAT, nbmat, FALSE);
   gcvt(maxdisp, 14, temp);
   SetDlgItemText(hDlg, AD_MAXDISP, temp);

  /* Joints */
   gcvt(jmat[0].fric, 14, temp);
   SetDlgItemText(hDlg, AD_FRIC, temp);
   gcvt(jmat[0].coh, 14, temp);
   SetDlgItemText(hDlg, AD_COH, temp);
   gcvt(jmat[0].tens, 14, temp);
   SetDlgItemText(hDlg, AD_TENS, temp);

  /* Material properties */
   gcvt(bmat[0].dens, 14, temp);
   SetDlgItemText(hDlg, AD_DENS, temp);
   gcvt(bmat[0].wt, 14, temp);
   SetDlgItemText(hDlg, AD_WT, temp);
   gcvt(bmat[0].ymod, 14, temp);
   SetDlgItemText(hDlg, AD_YMOD, temp);
   gcvt(bmat[0].pois, 14, temp);
   SetDlgItemText(hDlg, AD_POIS, temp);
   

  /*  Initialize the stress values. */
   for(i=0; i<3; i++) 
   {
      gcvt(bmat[0].iss[i], 14, temp);
      SetDlgItemText(hDlg, AD_ISSX+i, temp);
   }
   
  /* Initialize the strain values.  */
   for(i=0; i<3; i++) 
   {
      gcvt(bmat[0].ist[i], 14, temp);
      SetDlgItemText(hDlg, AD_ISTX+i, temp);
   }

  /*  The initial velocity values. */
   for(i=0; i<3; i++) 
   {
      gcvt(bmat[0].ivel[i], 14, temp);
      SetDlgItemText(hDlg, AD_IVELX+i, temp);
   }

   for(i=0; i<njmat; i++) 
   {
      SendDlgItemMessage(hDlg, AD_JMAT, CB_ADDSTRING, 0, (LPARAM) ((LPCSTR) numlist[i]));
   }
   SendDlgItemMessage(hDlg, AD_JMAT, CB_SETCURSEL, 0, 0);

   for(i=0; i<nbmat; i++) 
   {
      SendDlgItemMessage(hDlg, AD_BMAT, CB_ADDSTRING, 0, (LPARAM) ((LPCSTR) numlist[i]));
   }
   SendDlgItemMessage(hDlg, AD_BMAT, CB_SETCURSEL, 0, 0);

  /* Handle the time dependent stuff.
   * The next block gets the spinner.
   */
   for(i=0; i<numtdeps; i++) 
   {
      SendDlgItemMessage(hDlg, AD_TIMEDEPS, CB_ADDSTRING, 0, (LPARAM) ((LPCSTR) numlist[i]));
   }
   SendDlgItemMessage(hDlg, AD_TIMEDEPS, CB_SETCURSEL, 0, 0);
  /* Now set the scrollbar. */
   hLPScroller = GetDlgItem(hDlg, AD_TDSCROLL);
   scrollinfo.fMask = SIF_POS;
   
  /* We just set the scrollbar, now set the values in the visible 
   * part of the homegrown cell matrix widget.
   */
  /* FIXME:  segfaults here if new analysis menu item is selected.
   */
   if (lpoints == NULL)
      nlp = 0;
   else
      nlp = lpoints[0].loadpointsize1;

  /* Time dependent load values */
   //if (nlp > 0)
   //   setLoadCells(hDlg, 0, 0);

  /* Set the initial scroll bar position */ 
   if (nlp > 3)
      i = nlp-4;
   else 
      i = 0;
   
   SetScrollRange(hLPScroller, SB_CTL, 0, i, FALSE);
   SetScrollPos(hLPScroller, SB_CTL, 0, TRUE);
   
   if (nlp > 0)
      ; //  Do nothing for now: initBoltBoxes(hDlg);
   else 
      enableLoadBoxes(hDlg, FALSE);

}  /* Close setDialogValues()  */


static void
saveData()
{

   int i;
   FILE * fp;

   fp = fopen(filepath.apath, "w");
   //fp = fopen("testfile.ana", "w");
  /* Might want to reimplement time step saving interval
   * as well.
   */

   ad->analysistype = dynam;
   ad->nTimeSteps = nts;
   ad->nBlockMats = nbmat;
   ad->nJointMats = njmat;
   //ad->g2 = maxdisp;
   ad->maxdisplacement = maxdisp;
   ad->maxtimestep = .01;

   ad->rotationflag = rotation;
   ad->gravityflag = gravity;
   ad->planestrainflag = planestrain;
   ad->autotimestepflag = autotimestep;
   ad->autopenaltyflag = autopenalty;
   ad->gravaccel = 9.81;  // gravity needs to be a user defined quantity
   ad->nTDPoints = numtdeps;

   ad->materialpropsize1 = nbmat+1;
   ad->materialpropsize2 = 13;
   ad->materialProps = DoubMat2DGetMem(ad->materialpropsize1,ad->materialpropsize2);
 
   ad->phicohesionsize1 = njmat+1;
   ad->phicohesionsize2 = 3;
   ad->phiCohesion = DoubMat2DGetMem(ad->phicohesionsize1,ad->phicohesionsize2);

   for(i=0; i<nbmat; i++) 
   {
      ad->materialProps[i+1][0] = bmat[i].dens;
      ad->materialProps[i+1][1] = bmat[i].wt;
      ad->materialProps[i+1][2] = bmat[i].ymod;
      ad->materialProps[i+1][3] = bmat[i].pois;
      ad->materialProps[i+1][4] = bmat[i].iss[0];
      ad->materialProps[i+1][5] = bmat[i].iss[1];
      ad->materialProps[i+1][6] = bmat[i].iss[2];
      ad->materialProps[i+1][7] = bmat[i].ist[0];
      ad->materialProps[i+1][8] = bmat[i].ist[1];
      ad->materialProps[i+1][9] = bmat[i].ist[2];
      ad->materialProps[i+1][10] = bmat[i].ivel[0];
      ad->materialProps[i+1][11] = bmat[i].ivel[1];
      ad->materialProps[i+1][12] = bmat[i].ivel[2];
   }

   for(i=0; i<njmat; i++) 
   {
      ad->phiCohesion[i+1][0] = jmat[i].fric;
      ad->phiCohesion[i+1][1] = jmat[i].coh;
      ad->phiCohesion[i+1][2] = jmat[i].tens;
   }
      
  /* This is a really bad business here. 
   * FIXME: Time dependent points code needs to 
   * be rewritten very badly.  This probably
   * will not happen until the entire point
   * handling code gets rewritten.
   */
  /* Number of loading points will not change in 
   * analysis dialog.
   */
   //numtdeps = ad->nLPoints;
  /* Out with the old... */
  /* FIXME: write the memory handler for freeing loadpoints.
   if (ad->loadpoints)
      freeLoadPoints(ad);
   */
  /* in with the new. */
   ad->loadpoints = lpoints;

   ad->dump(ad->this,fp);
   //dumpAnalysisData(ad,fp);

  /* Leave this stuff for clean up. */
   if(jmatOld) 
      free(jmatOld);
   free(jmat);
   if(bmatOld)  
      free(bmatOld);
   free(bmat);
  /* FIXME: free analysis data struct also */
   fclose(fp);

} /* saveData() */


/* Eventually, setLoadPoints should handle all
 * instances of filling in cells.  In fact,
 * given a window handle, and parameters, it 
 * could probably be used from the geometry 
 * file as well.  scrollpos is the same as the 
 * global lpscrollpos.
 * FIXME: This segfaults when loading up an analysis 
 * file during run time.
 */
static void 
setLoadCells(HWND hDlg, int index, int scrollpos)
{
   int i,j;

  /* FIXME: New analysis crashes this because there are
   * no load points to check for.   
   */                                                                                           
   if (lpoints[index].loadpointsize1 < 4)
      j = lpoints[index].loadpointsize1;
   else 
      j = 4;

  /* FIXME: rewrite this in the same way the KILLFOCUS
   * is handled, provided that the kill focus works.
   */
   for (i=0;i<j;i++)
   {   
      gcvt(lpoints[index].vals[scrollpos+i][0], 14, temp);
      SetDlgItemText(hDlg, AD_STARTTIME1+3*i, temp);
      gcvt(lpoints[index].vals[scrollpos+i][1], 14, temp);
      SetDlgItemText(hDlg, AD_STARTTIME1+3*i+1, temp);
      gcvt(lpoints[index].vals[scrollpos+i][2], 14, temp);
      SetDlgItemText(hDlg, AD_STARTTIME1+3*i+2, temp);
   }

}  /* close  setLoadCells() */


/* The basic stuff here was copied from the geometry 
 * dialog.  If the analysis dialog needs to be extended
 * in the future, the geometry dialog code should be 
 * used as example for implementing extra scrollbars.
 */
static void
handleScrollBar(HWND hDlg, LPARAM lParam, WPARAM wParam, int lpoint)
{

   int np;

   hLPScroller = (HWND)lParam;

  /* If another scroller is added, we will have to use
   * this ID to figure out which scroller to process.
   * Example in geometry dialog.
   */
   //i = GetDlgCtrlID(hLPScroller);
  /* FIXME:  segfaults here if new analysis menu item is selected.
   */
   if (lpoints == NULL)
      np = 0;
   else
      np = lpoints[0].loadpointsize1;

   switch (LOWORD(wParam)) 
   {
      case SB_LINEDOWN: 
         lpscrollpos++; 
         break;

      case SB_LINEUP: 
         lpscrollpos--; 
         break;

      case SB_PAGEDOWN: 
         lpscrollpos += 5; 
         break;

      case SB_PAGEUP: 
         lpscrollpos -= 5; 
         break;

      case SB_THUMBTRACK: 
         lpscrollpos =  HIWORD(wParam); 
         break;
   }

   if(lpscrollpos > np-4) 
      lpscrollpos = np-4;

   if(lpscrollpos < 0) 
      lpscrollpos=0;
  
   if(lpscrollpos != GetScrollPos(hLPScroller, SB_CTL))
      SetScrollPos(hLPScroller, SB_CTL, lpscrollpos, TRUE);
   
  /* Kludge. Loadpoint handling is totally kludged.
   */
   if (np != 0)
      setLoadCells(hDlg, lpoint, lpscrollpos);

}  /* close handleScrollBar() */



static void 
enableLoadBoxes(HWND hDlg, BOOLEAN flag)
{
   int i;
   HWND hEditCtrl;

   /* This will need to be changed to allow 
    * i to start from a certain value.
    */
   for (i=0;i<13;i++)
   {
      hEditCtrl = GetDlgItem(hDlg, AD_TIMEDEPS+i);
      EnableWindow(hEditCtrl, flag);
   }

}  /* close enableLoadBoxes() */


static void 
handleJointPlus(HWND hDlg)
{
   int i;

   if(njmat < 20) 
   {
      if(jmatOld && jmatOld != jmat) 
         free(jmatOld);
      jmatOld = jmat;
      njmatOld = njmat;
      njmat++;
      jmat = (JointMat *) malloc(sizeof(JointMat) * njmat);
      for(i = 0; i<njmatOld; i++) 
      {
         jmat[i].fric = jmatOld[i].fric;
         jmat[i].coh = jmatOld[i].coh;
         jmat[i].tens = jmatOld[i].tens;
      }
        
      for(i = njmatOld; i<njmat; i++) 
      {
         jmat[i].fric = 0;
         jmat[i].coh = 0;
         jmat[i].tens = 0;
      }

      SetDlgItemInt(hDlg, AD_NJMAT, njmat, FALSE);
      SendDlgItemMessage(hDlg, AD_JMAT, CB_RESETCONTENT, 0, 0);

      for(i=0; i<njmat; i++) 
      {
         SendDlgItemMessage(hDlg, AD_JMAT, CB_ADDSTRING, 0, (LPARAM) ((LPCSTR) numlist[i]));
      }

      SendDlgItemMessage(hDlg, AD_JMAT, CB_SETCURSEL, 0, 0);
      gcvt(jmat[0].fric, 4, temp);
      SetDlgItemText(hDlg, AD_FRIC, temp);
      gcvt(jmat[0].coh, 4, temp);
      SetDlgItemText(hDlg, AD_COH, temp);
      gcvt(jmat[0].tens, 4, temp);
      SetDlgItemText(hDlg, AD_TENS, temp);
      jindex = 0;
   }  // end if
}  /* close handleJointPlus() */


static void
handleJointMinus(HWND hDlg)
{
   int i;

   if(njmat > 1) 
   {
      if(jmatOld && jmatOld != jmat) 
         free(jmatOld);
      jmatOld = jmat;
      njmatOld = njmat;
      njmat--;
      /* itoa probably needs to be replaced with 
       * sprintf 
       */
      itoa(njmat, temp, 10);
      SetDlgItemText(hDlg, AD_NJMAT, temp);
      SendDlgItemMessage(hDlg, AD_JMAT, CB_RESETCONTENT, 0, 0);

      for(i=0; i<njmat; i++) 
      {
         SendDlgItemMessage(hDlg, AD_JMAT, CB_ADDSTRING, 0, (LPARAM) ((LPCSTR) numlist[i]));
      }

      SendDlgItemMessage(hDlg, AD_JMAT, CB_SETCURSEL, 0, 0);
      gcvt(jmat[0].fric, 4, temp);
      SetDlgItemText(hDlg, AD_FRIC, temp);
      gcvt(jmat[0].coh, 4, temp);
      SetDlgItemText(hDlg, AD_COH, temp);
      gcvt(jmat[0].tens, 4, temp);
      SetDlgItemText(hDlg, AD_TENS, temp);
      jindex = 0;
   }  //end if
}  /* close handleJointMinus() */