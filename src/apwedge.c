
#define STRICT
#include<stdio.h>
#include<math.h>
#include<direct.h>
#include"ddamemory.h"
#include"resource.h"
#include <windows.h>
#include <windowsx.h>
#include "geometrydata.h"
#include "analysisdata.h"


#ifndef PI
#define PI 3.141592653
#endif
       
//static BOOLEAN handleWMCommand(HWND hDlg, LPARAM lParam, WPARAM wParam);
//static BOOLEAN handleInit(HWND);
static BOOLEAN handleSave(HWND);
static void changeJoint1(HWND hDlg, LPARAM lParam,WPARAM wParam);
static void changeJoint2(HWND hDlg, LPARAM lParam,WPARAM wParam);
static void changeJoint3(HWND hDlg, LPARAM lParam,WPARAM wParam);
static void handleAlpha(HWND hDlg, LPARAM lParam);
static void handleBeta(HWND hDlg, LPARAM lParam);
static void handlePsi(HWND hDlg, LPARAM lParam);
static void handleDelta(HWND hDlg, LPARAM lParam);
static BOOLEAN setDialogValues(HWND);

/* Push these down low */
static void updatePsi();
static void updateDelta();
static void updateBeta();
static void updateAlpha();
static void updateB();
static void updateC();
static void updateD();

static void updateGeometry();
static void updateJoints();
static void updateFpoints();
static void updateMpoints();
static void findCentroids();

/* "Primary" angles, angles the user can change. */
static double alpha,beta,psi,delta;
/* Secondary angles used internally. */
static double sigma, tau, theta, gamma;
/* Unit height */
static double H = 1.0;
/* Side lengths, needed internally */
static double AB, AC, BC, BD;
/* Internal heights */
static double h1, h2;
/* friction, cohesion and tensile strength
 * for joints 1, 2 and 3.
 */
static double jf1, jf2, jf3, jc1, jc2, jc3, jt1, jt2, jt3;
/* Change to decimal degrees */
const double DD = PI/180;
char temp[80];
char path[256];
char * dirnameptr;

struct {
   double x,y;
} A,B,C,D;


static Analysisdata *  adata;
static Geometrydata * gdata;


/* This function also needs to handle initializing 
 * the edit boxes in the dialog box.
 */
static BOOLEAN
handleInit(HWND hDlg)
{

  /* Primary angles */
   alpha = 90;
   beta = 60;
   psi = 65;
   delta = 5;

  /* some friction */
   jf1 = 30;
   jf2 = 25;
   jf3 = 35;

  /* some cohesion (not used) */
   jc1 = 0;
   jc2 = 0;
   jc3 = 0;
   
  /* and some tensile strength (not used) */
   jt1 = 0;
   jt2 = 0;
   jt3 = 0; 


  /* Initialize secondary angles */
   sigma = psi - delta;
   tau = 180 - sigma - delta;
   theta = 180 - psi - tau;
   gamma = 180 - theta - alpha;

  /* Fix the first vertex A */
   A.x = 1.0;
   A.y = 1.0;
   B.y = 2.0;
   D.y = 2.0;

  /* Grab Geometrydata and Analysisdata structs...
   */
   adata = adata_new();
   //gdata = initGeometrydata();
   gdata = gdata_new();

   adata->gravityflag = 0;
   adata->autotimestepflag = 1;
   adata->autopenaltyflag = 1;
   adata->planestrainflag = 0;
   adata->analysistype = 0;  // STATIC
   adata->nTDPoints = 3;
   adata->nTimeSteps = 100;
   adata->maxtimestep = .01;
   //adata->maxDisplacement = .01;
   //adata->g2 = 0.01;
   adata->maxdisplacement = 0.01;
   adata->gravaccel = 9.81;  // constant grav ok here
   adata->nBlockMats = 1;
   adata->nJointMats = 4;

   gdata->e00 = .01;
   gdata->nJoints = 9;
   gdata->nFPoints = 3;
   gdata->nMPoints = 2;
   gdata->nLPoints = 0;
   gdata->nHPoints = 0;  // Hole points

   gdata->jointsize1 = gdata->nJoints + 1;  // joints[0][i] not used
   gdata->jointsize2 = 6;
   gdata->joints = DoubMat2DGetMem(gdata->jointsize1,gdata->jointsize2);
  /* Fixedpoints and measured points are really messy.  This code 
   * uses geometry reading code for prototype.
   */

   // mmm: this should probably be set somewhere else, but I don't know where
   // is there a file that contains hardwired parameters?
   gdata->maxFixedPointsPerFixedLine = 100; // hardwired
   gdata->pointsize1 = (gdata->nFPoints*(gdata->maxFixedPointsPerFixedLine+1))+gdata->nLPoints+gdata->nMPoints+gdata->nHPoints+1;

   gdata->pointsize2 = 9;  /* add cum disp in [][7] and [][8]  */
   gdata->points = DoubMat2DGetMem(gdata->pointsize1, gdata->pointsize2);
  
   adata->materialpropsize1 = adata->nBlockMats + 1;
   adata->materialpropsize2 = 13;
   adata->materialProps = DoubMat2DGetMem(adata->materialpropsize1,adata->materialpropsize2);

  /* Load some default material properties... */
   adata->materialProps[1][0] = 10.0;
   adata->materialProps[1][1] = 100.0;
   adata->materialProps[1][2] = 1000000.0;
   adata->materialProps[1][3] = .29;

   adata->phicohesionsize1 = adata->nJointMats + 1;
   adata->phicohesionsize2 = 3;
   adata->phiCohesion = DoubMat2DGetMem(adata->phicohesionsize1,adata->phicohesionsize2);

  /* Lets get some initial data loaded up for testing */
   updatePsi();

   setDialogValues(hDlg);

  /* Check the path. If we aren't in apwedge directory,
   * go there, or make one and go there.
   */
   _getcwd(path,256);
   dirnameptr = strrchr(path,'\\');
  /* Pay attention here: First we got the complete directory
   * name with path prefixed.  Then we grabbed a pointer the 
   * _last '\' in the path, which is followed by the directory
   * name. Now we don't really care about the '\', so we 
   * increment the pointer to the character following, which is 
   * the first letter of the path.  Also, for string matching, 
   * we have to match against the null value terminating the 
   * string, or else have to count characters and do a strncmp
   * (which would probably be safer anyway.
   */
   MessageBox(hDlg, dirnameptr+1,"blah",IDOK);
   if (!strcmp(dirnameptr+1,"apwedge\0")) exit(0);

   if (_chdir("./apwedge"))
   {
     _mkdir("./apwedge");
     _chdir("./apwedge");
   }
   return 0;

}  /* close handleInit() */



/* Write out the analysis and geometry data into XML 
 * input files.
 */
BOOLEAN
handleSave(HWND hDlg)
{
  /* Temporary output... */
   FILE * ofp;

  /* write a geometry file */
   ofp = fopen("apwedge.geo","w");
   gdata->dumptofile(gdata, fprintf, ofp);
   fclose(ofp);

  /* write an analysis file */

  /* Won't link, don't know why. */


   ofp = fopen("apwedge.ana","w");
   adata_write_ddaml(adata,(PrintFunc)fprintf,ofp);
   fclose(ofp);

   //EndDialog (hDlg, 0);   // return 0: don't save data

   return 0;

}  /* close handleSave() */

         
         
static void
changeJoint1(HWND hDlg, LPARAM lParam,WPARAM wParam)
{

   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L))
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);

         GetDlgItemText(hDlg, APWEDGE_JF1, temp, 20);
        /* FIXME: atof should be changed to strtod, but I am 
         * not sure what win32 uses for end marker.  
         */
         jf1 = atof(temp);
         adata->phiCohesion[1][0] = jf1;

         GetDlgItemText(hDlg, APWEDGE_JC1, temp, 20);
         jc1 = atof(temp);
         adata->phiCohesion[1][1] = jc1;

         GetDlgItemText(hDlg, APWEDGE_JT1, temp, 20);
         jt1 = atof(temp);
         adata->phiCohesion[1][1] = jt1;

      } // end if

   } // end if

}  /* changeJoint1() */
         
static void
changeJoint2(HWND hDlg, LPARAM lParam,WPARAM wParam)
{

   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L))
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);

         GetDlgItemText(hDlg, APWEDGE_JF2, temp, 20);
         jf2 = atof(temp);
         adata->phiCohesion[2][0] = jf2;

         GetDlgItemText(hDlg, APWEDGE_JC2, temp, 20);
         jc2 = atof(temp);
         adata->phiCohesion[2][1] = jc2;

         GetDlgItemText(hDlg, APWEDGE_JT2, temp, 20);
         jt2 = atof(temp);
         adata->phiCohesion[2][2] = jt2;

      } // end if

   } // end if

}  /* changeJoint2() */
         

static void
changeJoint3(HWND hDlg, LPARAM lParam,WPARAM wParam)
{

   if(HIWORD(wParam) == EN_KILLFOCUS) 
   {
      //MessageBox(hDlg, "Got EN_KILLFOCUS", "WM_COMMAND", MB_OK);
      if (SendMessage( (HWND) lParam,  EM_GETMODIFY, 0, 0L))
      {
         SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);

         GetDlgItemText(hDlg, APWEDGE_JF3, temp, 20);
         jf3 = atof(temp);
         adata->phiCohesion[3][0] = jf3;

         GetDlgItemText(hDlg, APWEDGE_JC3, temp, 20);
         jc3 = atof(temp);
         adata->phiCohesion[3][1] = jc3;

         GetDlgItemText(hDlg, APWEDGE_JT3, temp, 20);
         jt3 = atof(temp);
         adata->phiCohesion[3][2] = jt3;

      } // end if

   } // end if

}  /* changeJoint3() */


static BOOLEAN 
setDialogValues(HWND hDlg)
{

   char temp[25];

   sprintf(temp,"%3.2f",alpha);
   SetDlgItemText(hDlg, APWEDGE_A1, temp);

   sprintf(temp,"%3.2f",beta);
   SetDlgItemText(hDlg, APWEDGE_A2, temp);

   sprintf(temp,"%3.2f",psi);
   SetDlgItemText(hDlg, APWEDGE_A3, temp);

   sprintf(temp,"%3.2f",delta);
   SetDlgItemText(hDlg, APWEDGE_A4, temp);

   sprintf(temp,"%3.2f",jf1);
   SetDlgItemText(hDlg, APWEDGE_JF1, temp);
   adata->phiCohesion[1][0] = jf1;

   sprintf(temp,"%3.2f",jf2);
   SetDlgItemText(hDlg, APWEDGE_JF2, temp);
   adata->phiCohesion[2][0] = jf2;

   sprintf(temp,"%3.2f",jf3);
   SetDlgItemText(hDlg, APWEDGE_JF3, temp);
   adata->phiCohesion[3][0] = jf3;

   return TRUE;

}  /* close setDialogValues() */



static void
handleAlpha(HWND hDlg, LPARAM lParam)
{
   SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
   GetDlgItemText(hDlg, APWEDGE_A1, temp, 20);
   alpha = strtod(temp, NULL);
   updateAlpha();
}  /* close handleAlpha() */


static void
handleBeta(HWND hDlg, LPARAM lParam)
{
   SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
   GetDlgItemText(hDlg, APWEDGE_A2, temp, 20);
   beta = strtod(temp, NULL);
   updateBeta();
}  /* close handleBeta() */


static void
handlePsi(HWND hDlg, LPARAM lParam)
{
   SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
   GetDlgItemText(hDlg, APWEDGE_A3, temp, 20);
   psi = strtod(temp, NULL);
   updatePsi();
}  /* close handlePsi() */


static void
handleDelta(HWND hDlg, LPARAM lParam)
{
   SendMessage( (HWND) lParam, EM_SETMODIFY, FALSE, 0L);
   GetDlgItemText(hDlg, APWEDGE_A4, temp, 20);
   delta = strtod(temp, NULL);
   updateDelta();
}  /* close handleDelta() */



static void
updatePsi()
{
  /* Check to make sure psi > delta */

   sigma = psi - delta;
   tau = 180 - sigma - delta;
   theta = 180 - psi - tau;
   gamma = 180 - theta - alpha;
 
   updateB();

}  /* close updatePsi() */
static void
updateDelta()
{
   sigma = psi - delta;
   tau = 180 - sigma - delta;
   theta = 180 - psi - tau;
   gamma = 180 - theta - alpha;

   updateC();

}  /* close updateDelta() */
static void
updateBeta()
{

   tau = 180 - sigma - delta;
   theta = 180 - psi - tau;
   gamma = 180 - theta - alpha;

   updateC();

}  /* close updateBeta() */

static void
updateAlpha()
{
   theta = 180 - psi - tau;
   gamma = 180 - theta - alpha;

   updateD();
    
}  /* close updateAlpha() */

static void
updateB()
{
  /* Recompute vertex B location here */

   AB = H/sin(psi*DD);
   B.x = A.x + AB*cos(psi*DD);
   B.y = 2.0; //A.y + AB*sin(psi*DD);
  
  /* Then update the next vertex */
   updateC();

}  /* close updateB() */

static void
updateC()
{

  /* Recompute vertex C location */
   h1 = AB*sin(sigma*DD);
   AC = AB*cos(sigma*DD) + h1/tan(beta*DD);
   C.x = AC*cos(delta*DD) + A.x;
   C.y = AC*sin(delta*DD) + A.y;

  /* Then update D */
   updateD();

}  /* close updateC() */

static void
updateD()
{

   /* Update vertex D location */
    BC = AB*(sin(sigma*DD)/sin(beta*DD));

    h2 = BC*sin(theta*DD);
    BD = BC*cos(theta*DD) + h2/tan(gamma*DD);
    D.x = B.x + BD;
    D.y = B.y;

   /* Now update all the joints */
    updateGeometry();
    
}  /* close updateD() */

static void
updateGeometry()
{

   updateJoints();
   updateFpoints();

}  /* close updateGeometry() */

/* Now we have all of the vertices, so we need to 
 * update the geometry structure.
 */
static void
updateJoints()
{
   double ** joints = gdata->joints;

  /* Update the geometry structure... */
   joints[1][1] = C.x;
   joints[1][2] = C.y;
   joints[1][3] = D.x;
   joints[1][4] = D.y;
   joints[1][5] = 1; // type

   joints[2][1] = A.x;
   joints[2][2] = A.y;
   joints[2][3] = C.x;
   joints[2][4] = C.y;
   joints[2][5] = 2; // type

   joints[3][1] = B.x;
   joints[3][2] = B.y;
   joints[3][3] = C.x;
   joints[3][4] = C.y;
   joints[3][5] = 3; // type

   joints[4][1] = A.x;
   joints[4][2] = A.y;
   joints[4][3] = 0;
   joints[4][4] = 1;
   joints[4][5] = 2; // type

   joints[5][1] = 0;
   joints[5][2] = 1;
   joints[5][3] = 0;
   joints[5][4] = 0;
   joints[5][5] = 4;

   joints[6][1] = 0;
   joints[6][2] = 0;
   joints[6][3] = D.x+1;
   joints[6][4] = 0;
   joints[6][5] = 4;

   joints[7][1] = D.x+1;
   joints[7][2] = 0;
   joints[7][3] = D.x+1;
   joints[7][4] = D.y;
   joints[7][5] = 4;

   joints[8][1] = D.x+1;
   joints[8][2] = D.y;
   joints[8][3] = B.x;
   joints[8][4] = B.y;
   joints[8][5] = 4;

   joints[9][1] = A.x;
   joints[9][2] = A.y;
   joints[9][3] = B.x;
   joints[9][4] = B.y;
   joints[9][5] = 4;


  /* Then recompute the problem domain size, 
   * fixed point locations, etc.
   */

}  /* close updateJoints() */

static void
updateFpoints()
{
   double ** points = gdata->points;

  /* Fixed points go in points[1][] to points[nFPoints][] */
   points[1][1] = 0;
   points[1][2] = 0;
   points[1][3] = 0;
   points[1][4] = 0;

   points[2][1] = D.x+1;
   points[2][2] = 0;
   points[2][3] = D.x+1;
   points[2][4] = 0;

   points[3][1] = D.x+1;
   points[3][2] = D.y;
   points[3][3] = D.x+1;
   points[3][4] = D.y;

}  /* close updateFpoints() */


static void
updateMpoints()
{
  int i= gdata->nFPoints+gdata->nLPoints+1;
  double ** points = gdata->points;

  /* Have to find centroids of the two blocks. */
   findCentroids();

  /* measured points are after load points.  We don't 
   * have any load points in here (yet).
   */
  /* Block 1 */
   points[i][1] = 0;  /// temp
   points[i][2] = 0;
  /* Block 2 */
   points[i+1][1] = 0;
   points[i+1][2] = 0;
  
    
}  /* close updateMpoints(0 */


static void
findCentroids()
{


}  /* close findCentroids() */

static BOOLEAN
handleWMCommand(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
   switch (LOWORD(wParam))
   {
      case APWEDGE_SAVE:
         return(handleSave(hDlg));

      case IDCANCEL:
        /* FIXME: need to free mem before calling EndDialog() */
         EndDialog (hDlg, 0);   // return 0: don't save data
         return TRUE;

      case APWEDGE_JF1:
      case APWEDGE_JC1:
      case APWEDGE_JT1:
         changeJoint1(hDlg,lParam,wParam);
         break;

      case APWEDGE_JF2:
      case APWEDGE_JC2:
      case APWEDGE_JT2:
         changeJoint2(hDlg,lParam,wParam);
         break;

      case APWEDGE_JF3:
      case APWEDGE_JC3:
      case APWEDGE_JT3:
         changeJoint3(hDlg,lParam,wParam);
         break;

      case APWEDGE_A1:
         handleAlpha(hDlg, lParam);
         break;

      case APWEDGE_A2:
         handleBeta(hDlg, lParam);
         break;

      case APWEDGE_A3:
         handlePsi(hDlg, lParam);
         break;

      case APWEDGE_A4:
         handleDelta(hDlg, lParam);
         break;

   } /* end switch wmcommand */

  /* FIXME: ensure that this should return true here.
   */
   return TRUE;

} /* close handleWMCommand() */

BOOL CALLBACK  
APWEDGEDlgProc (HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
   switch (iMessage)
   {
      case WM_INITDIALOG :
         return(handleInit(hDlg));     
    
      case WM_COMMAND:
         return(handleWMCommand(hDlg, lParam, wParam));

   } // end switch (imessage)

   return FALSE ;

}  // end GeomDlgProc


