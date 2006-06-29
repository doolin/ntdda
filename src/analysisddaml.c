
/*
 * analysisddaml.c 
 *
 * An example program for DDA analysis input file  XML format.
 *
 * Copyright: http://www.gnu.org/copyleft/lesser.html
 *
 * David M. Doolin  doolin@ce.berkeley.edu
 *
 * $Author: doolin $
 * $Date: 2006/06/29 20:44:32 $
 * $Source: /cvsroot/dda/ntdda/src/analysisddaml.c,v $
 * $Revision: 1.26 $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "ddaml_private.h"
#include "ddaml.h"
#include "ddamemory.h"
#include "analysisdata.h"
#include "timehistory.h"

/** Get rid of this later. 
 * The way to get rid of it is to define 
 * ddaml_display_* functions, then set them
 * to the dda_display_* functions.
 */
#include "dda.h"
#include "bolt.h"
#include "joint.h"


/* Mostly for debugging windows. */
static char mess[128];



typedef DList LOADPOINTLIST;
typedef DList JOINTMATLIST;
typedef DList BOLTMATLIST; 
typedef DList BLOCKMATLIST; 

static LOADPOINTLIST * loadpointlist;
static JOINTMATLIST * jointmatlist;
static BLOCKMATLIST * blockmatlist;
static BOLTMATLIST * boltmatlist;

static xmlNsPtr nspace;


static Jointmat * getNewJointMat(void);

static BlockMat * getNewBlockMat(void);

void transferAData(void);
void transferJointMatlistToAStruct(Analysisdata * adata, JOINTMATLIST * jointmatlist);
void transferLoadpointlistToAStruct(Analysisdata * adata, LOADPOINTLIST * loadpointlist);
void transferBoltMatlistToAStruct(Analysisdata * adata, BOLTMATLIST * boltmatlist);
void transferBlockMatlistToAStruct(Analysisdata * adata, BLOCKMATLIST * blockmatlist);
static double ** DoubMat2DGetMem(int n, int m);


static void initializeALists(void);

Analysisdata * adata;



void 
initializeALists()
{
   jointmatlist = dlist_new();
   blockmatlist = dlist_new();
   boltmatlist = dlist_new();
   loadpointlist = dlist_new();

}  /* close initializeLists() */


void 
transferAData(void)
{

   transferJointMatlistToAStruct(adata,jointmatlist);
   transferLoadpointlistToAStruct(adata, loadpointlist);
   transferBoltMatlistToAStruct(adata,boltmatlist);
   transferBlockMatlistToAStruct(adata, blockmatlist);
}  /* close transferAData() */


void 
transferJointMatlistToAStruct(Analysisdata * ad, JOINTMATLIST * jointmatlist)
{
   int i = 0;
   int njmat;
   JOINTMATLIST * ptr;
   Jointmat * jmtmp;

  /* WARNING: Type attribute of xml tag is ignored for now.
   * Joint materials should be listed in order of occurrence
   * in the xml file.
   */
   njmat = dlist_length(jointmatlist);
   ad->nJointMats = njmat;
   ad->phicohesionsize1 = njmat + 1;
   ad->phicohesionsize2 = 3;
   ad->phiCohesion = DoubMat2DGetMem(adata->phicohesionsize1,adata->phicohesionsize2);

  /* WARNING: This assumes that joint types are listed in order 
   * of occurrence in xml file.  The type attribute is ignored
   * for now.
   */
   dlist_traverse(ptr, jointmatlist)
   {
      jmtmp = ptr->val;
      ad->phiCohesion[i+1][0] = jointmat_get_friction(jmtmp);//->fric;
      ad->phiCohesion[i+1][1] = jointmat_get_cohesion(jmtmp);//->coh; 
      ad->phiCohesion[i+1][2] = jointmat_get_tension(jmtmp);//->tens;
      i++;
   }
    
}  /*  close transferJointMatListToAStruct() */


void 
transferLoadpointlistToAStruct(Analysisdata * ad, LOADPOINTLIST * loadpointlist)
{
   int i = 0;
   int s1,s2;
   double ** lp;
   
   Loadpoint * lptmp;
   LOADPOINTLIST * ptr;
  
   ad->nLPoints = dlist_length(loadpointlist);

   if (ad->nLPoints == 0)
      return;

   //ad->loadpoints = (Loadpoint *) calloc (ad->nLPoints, sizeof (Loadpoint));
   ad->loadpoints = loadpoint_new_array(ad->nLPoints);

  /* WARNING: This assumes that load points are listed in order 
   * of occurrence in xml file.  
   */
   dlist_traverse(ptr, loadpointlist)
   {
      lptmp = ptr->val;
		s1 = lptmp->loadpointsize1;
		s2 = lptmp->loadpointsize2;
      lp = lptmp->vals;

      ad->loadpoints[i].loadpointsize1 = s1;
      ad->loadpoints[i].loadpointsize2 = s2;
      ad->loadpoints[i].vals = clone2DMatDoub(lp, s1, s2);
      i++;
   }
   
}  /*  close transferLoadPointlistToAStruct() */



static void 
transferBoltMatlistToAStruct(Analysisdata * ad, BOLTMATLIST * boltmatlist)
{

   int i = 0;
   int nbmat;
   double e00, t0, f0;
   BOLTMATLIST * ptr;
   Boltmat * bmtmp;
  /* WARNING: Type attribute of xml tag is ignored for now.
   * Bolt materials should be listed in order of occurrence
   * in the xml file.
   */
   nbmat = dlist_length(boltmatlist);
   if (nbmat == 0) {
      return;
   }

   ad->nBoltMats = nbmat;
   ad->boltmatsize1 = nbmat;
   ad->boltmatsize2 = 3;
   ad->boltmats = DoubMat2DGetMem(adata->boltmatsize1,adata->boltmatsize2);


   dlist_traverse(ptr, boltmatlist) {
      bmtmp = ptr->val;
      boltmat_get_props(bmtmp,&e00,&t0,&f0);
      ad->boltmats[i][0] = e00;
      ad->boltmats[i][1] = t0; 
      ad->boltmats[i][2] = f0;
      i++;
   }

}  /*  close transferBoltMatlistToAStruct() */


void 
transferBlockMatlistToAStruct(Analysisdata * ad, BLOCKMATLIST * blockmatlist)
{
   int i = 0;
   int nbmat;
   BLOCKMATLIST * ptr;
   BlockMat * bmtmp;

  /* WARNING: Type attribute of xml tag is ignored for now.
   * Joint materials should be listed in order of occurrence
   * in the xml file.
   */
   nbmat = dlist_length(blockmatlist);
   ad->nBlockMats = nbmat;
   ad->materialpropsize1 = nbmat + 1;
   ad->materialpropsize2 = 14;
   ad->materialProps = DoubMat2DGetMem(adata->materialpropsize1,adata->materialpropsize2);

  /* WARNING: This assumes that joint types are listed in order 
   * of occurrence in xml file.  The type attribute is ignored
   * for now.
   */
   dlist_traverse(ptr, blockmatlist)
   {
      bmtmp = ptr->val;
      ad->materialProps[i+1][0] = bmtmp->dens;
      ad->materialProps[i+1][1] = bmtmp->wt; 
      ad->materialProps[i+1][2] = bmtmp->ymod;
      ad->materialProps[i+1][3] = bmtmp->pois;
      ad->materialProps[i+1][4] = bmtmp->iss[0];
      ad->materialProps[i+1][5] = bmtmp->iss[1];
      ad->materialProps[i+1][6] = bmtmp->iss[2];
      ad->materialProps[i+1][7] = bmtmp->ivel[0];
      ad->materialProps[i+1][8] = bmtmp->ivel[1];
      ad->materialProps[i+1][9] = bmtmp->ivel[2];
      ad->materialProps[i+1][10] = bmtmp->ist[0];
      ad->materialProps[i+1][11] = bmtmp->ist[1];
      ad->materialProps[i+1][12] = bmtmp->ist[2];
      ad->materialProps[i+1][13] = bmtmp->damping;
      i++;
   }
}  /*  close transferBlockMatlistToAStruct() */



static void 
parseRotation(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
  /* Need to grab a tempstring, then switch for different 
   * types of rotation,
   */
   char * temp;

   fprintf(stdout,"Parsing rotation...\n");
   
   temp = xmlGetProp(cur,"type");

   if (!strcmp(temp, "linear")) {
      adata->rotationflag = 0;
   } else if (!strcmp(temp, "secondorder")) {
      ddaml_display_warning("Using exact instead of second order");
      adata->rotationflag = 1;
   } else if (!strcmp(temp, "exact")) {
      adata->rotationflag = 1;
   } else {
      ddaml_display_warning("Parse problem in rotation attributes");
   }

}  


void 
parseGravity(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

   fprintf(stdout,"Parsing gravity...\n");

  /* This will be a struct at some point */
   if (!strcmp("yes",xmlGetProp(cur,"flag")) )
   {
      Gravity * grav;
     /* Set the flag: */
      adata->gravityflag = 1;
     /* Collect the parameters controlling the 
      * the gravity time steps, convergence, etc. 
      */
      grav  = newGravity();  //(Gravity *)calloc(1,sizeof(Gravity));
     
      fprintf(stderr,"Inside gravity\n");

      cur = cur->children;

      while (cur != NULL) 
      {
         if ((!strcmp(cur->name, "Gravitysteplimit")) )// && (cur->ns == ns))
            grav->gravTurnonLimit = atoi(xmlGetProp(cur,"val"));
         if ((!strcmp(cur->name, "Normaltolerance")) ) //&& (cur->ns == ns))
            grav->nforcetolerance = atof(xmlGetProp(cur,"val"));
         if ((!strcmp(cur->name, "Sheartolerance")) ) //&& (cur->ns == ns))
            grav->sforcetolerance = atof(xmlGetProp(cur,"val"));
         if ((!strcmp(cur->name, "Residualtolerance")) ) //&& (cur->ns == ns))
            grav->residual_tol = atof(xmlGetProp(cur,"val"));
         cur = cur->next;

      }  /* end loop over current pointer */

      adata->gravity = grav;
   }
   else
   {
      adata->gravityflag = 0;
   }

   fprintf(stdout,"gravity flag: %d\n\n", adata->gravityflag);

}  /*close parseGravity() */


void 
parseAutotimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   fprintf(stdout,"Parsing auto time step...\n");

  /* This will be a struct at some point */
   if (!strcmp("yes",xmlGetProp(cur,"flag")) )
   {
     /* Set the flag: */
      adata->autotimestepflag = 1;
   }
   else
   {
      adata->autotimestepflag = 0;
   }

   fprintf(stdout,"autotimestep flag: %d\n\n", adata->autotimestepflag);

}  /*close parseAutoTimestep() */


void 
parseAnalysistype(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
  
  /* This will be a struct at some point */
   if (!strcmp("dynamic",xmlGetProp(cur,"type")) )
   {
     /* Set the flag: */
      adata->analysistype = 1;
   }
   else
   {
      adata->analysistype = 0;
   }

}  /*close parseAnalysistype() */

void 
parseNumtimesteps(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

   adata->nTimeSteps = atoi(xmlGetProp(cur,"timesteps"));


}  /*close parseNumtimesteps() */


void 
parseAutopenalty(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   char * pfactor;
   fprintf(stdout,"Parsing auto penalty flag...\n");

  /* This will be a struct at some point */
   if (!strcmp("yes",xmlGetProp(cur,"flag")) )
   {
      adata->autopenaltyflag = 1;
   }
   else
   {
      adata->autopenaltyflag = 0;
   }

  /* Set this to zero in case the analysis file is not
   * updated for handling the pfactor attribute.  Then
   * the entire block system sinks.
   */
   adata->pfactor = 0.0;
   pfactor = xmlGetProp(cur,"pfactor");

   if (pfactor == NULL)
   {
      //sprintf(mess,"Warning","Penalty factor missing from Autopenalty element");
      //dda_display_warning(mess);
      dda_display_warning("Penalty factor missing from Autopenalty element");

   }
   else 
   {
      adata->pfactor = atof(pfactor);
   }

   fprintf(stdout,"autopenalty flag: %d\n\n", adata->autopenaltyflag);

}  /*close parseAutopenalty() */



void 
parsePlanestrain(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{

   fprintf(stdout,"Parsing planestrain flag...\n");

  /* This will be a struct at some point */
   if (!strcmp("yes",xmlGetProp(cur,"flag")) )
   {
     /* Set the flag: */
      adata->planestrainflag = 1;
   }
   else
   {
      adata->planestrainflag = 0;
   }

   fprintf(stdout,"planestrain flag: %d\n\n", adata->planestrainflag);



}  /*close parsePlanestrain() */


void 
parseAConstants(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
  /* constants needs to be changed into an instance 
   * variable.
   */
   //CONSTANTS * constants;
   //constants  = (CONSTANTS *)calloc(1,sizeof(CONSTANTS));
  
   Constants * constants = constants_new_defaults();


   fprintf(stderr,"Inside constants\n");

   cur = cur->children;

   while (cur != NULL) 
   {

      if (!strcmp(cur->name, "Openclose")) {
        //constants->openclose = atof(xmlGetProp(cur,"value"));
        constants_set_openclose(constants,atof(xmlGetProp(cur,"value")));
      }

      if (!strcmp(cur->name, "Opencriteria")) {
        //constants->opencriteria = atof(xmlGetProp(cur,"value"));
        constants_set_opencriteria(constants,atof(xmlGetProp(cur,"value")));
      }

     /* w0 gets reset during the analysis.  Probably need to turn this
      * into a factor of w0 to see if that helps scaling issues in 
      * problems with vastly different block sizes.
      */
      /*
      if (!strcmp(cur->name, "Domainscale")) //&& (cur->ns == ns))
        constants->w0 = atof(xmlGetProp(cur,"value"));
       */

      /* The following three are derived from the other five */
      
      if (!strcmp(cur->name, "NormSpringPen")) {  //&& (cur->ns == ns)) {

        //constants->norm_spring_pen = atof(xmlGetProp(cur,"value"));
        constants_set_norm_spring_pen(constants,atof(xmlGetProp(cur,"value")));
       /* if this assert fires, check the attribute value in the 
        * given tag.
        */
        //assert(constants->norm_spring_pen > 0);
      }

     /*
      if (!strcmp(cur->name, "NormExternDist")) //&& (cur->ns == ns))
        constants->norm_extern_dist = atof(xmlGetProp(cur,"value"));

      if (!strcmp(cur->name, "NormPenDist")) //&& (cur->ns == ns))
        constants->norm_pen_dist = atof(xmlGetProp(cur,"value"));
      */

      if (!strcmp(cur->name, "AngleOverlap")) // && (cur->ns == ns))
        constants_set_angle_olap(constants,atof(xmlGetProp(cur,"value")));

      if (!strcmp(cur->name, "ShearNormRatio")) // && (cur->ns == ns))
        constants_set_shear_norm_ratio(constants,atof(xmlGetProp(cur,"value")));

     /* MinReflineFactor is scaled by the domain size. Probably do not
      * need this factor at all.
      */
      if (!strcmp(cur->name, "MinReflineFactor")) {// && (cur->ns == ns))
        constants_set_min_refline_factor(constants,atof(xmlGetProp(cur,"value")));
      }

      //iface->displaymessage("In constants parsing code");
      cur = cur->next;

   }  /* end loop over current pointer */

   adata->constants = constants;   


}  /*close parseAConstants() */



void 
parseOpencloselimit(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   adata->OCLimit = atoi(xmlGetProp(cur,"maxopenclose"));
}  /*close parseOpencloselimit() */



void 
parseTimehistory(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   char * filename;
   char * fileformat;

   filename = xmlGetProp(cur,"filename");

   fileformat = xmlGetProp(cur,"format");

   if (!strcmp(fileformat,"shake"))
      th_read_data_file(adata->timehistory,filename,shake);
   else if (!strcmp(fileformat,"matlab"))
      th_read_data_file(adata->timehistory,filename,matlab);
   else // blow up
      dda_display_error("Bad motion file format");
} 


void 
parseGravaccel(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {

   char * grav_val = xmlGetProp(cur,"value");

   if (grav_val != NULL) {
      adata_set_grav_accel(adata,atof(grav_val));
   } else {
      adata_set_grav_accel(adata,9.81);
   }

}


void 
parseSaveinterval(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
   char * step;

   fprintf(stdout,"Parsing save interval...\n");

   step = xmlGetProp(cur,"step");
   
   if (step != NULL) {
      adata->tsSaveInterval = atoi(step);
   } else {
     /* Set default value of 10 */
      adata->tsSaveInterval = 10;
     /* And fire off a warning about bad attribute */
     /* FIXME: Handle warning. */
   }

   /* atoi _never_ fails */
   if (adata->tsSaveInterval == 0)
      adata->tsSaveInterval = 10;


}  /*close parseSaveinterval() */


void 
parseMaxtimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
   fprintf(stdout,"Parsing max time step...\n");
   adata->maxtimestep = atof(xmlGetProp(cur,"maxtimestep"));
}  


void 
parseMintimestep(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
   fprintf(stdout,"Parsing max time step...\n");
   adata->min_delta_t = atof(xmlGetProp(cur,"mintimestep"));
}  


void 
parseMaxdisplacement(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
   fprintf(stdout,"Parsing max displacement...\n");
   adata->maxdisplacement = atof(xmlGetProp(cur,"maxdisplacement"));
}  


void 
parseContactDamping(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
   double cn = atof(xmlGetProp(cur,"value"));
   adata_set_contact_damping(adata, cn);
}  

void 
parseJointproperties(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   Jointmat * jmat;
   double temp;
   cur = cur->children;
   jmat = jointmat_new();

  /** @todo Reimplement namespace checking.
   */
   while (cur != NULL) {

      if ((!strcmp(cur->name, "Friction")) ) {
         temp = atof(xmlNodeListGetString(doc, cur->children, 1));
         jointmat_set_friction(jmat,temp);
      }

      if ((!strcmp(cur->name, "Cohesion")) ) {
         temp = atof(xmlNodeListGetString(doc, cur->children, 1));
         jointmat_set_cohesion(jmat,temp);
      }

      if ((!strcmp(cur->name, "Tensile")) ) {
         temp = atof(xmlNodeListGetString(doc, cur->children, 1));
         jointmat_set_tension(jmat,temp);
      }

      cur = cur->next;
   }

   dl_insert_b(jointmatlist,(void*)jmat);


}  /*close parseJointproperties() */


void 
parseBlockmaterial(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
   BlockMat * bmat;
   double temp[3] = {0.0};
   char * tempstring;
   int checkval;  //  Use this to make sure sscanf got enough numbers

   static int gotdamping = 0;

   cur = cur->children;
  /* Grab a struct for block materials */
   bmat = getNewBlockMat(); 

  /** @todo Reimplement namespace checking. */
   while (cur != NULL) {

      if ((!strcmp(cur->name, "Unitmass")) ) //&& (cur->ns == ns))
         bmat->dens = atof(xmlNodeListGetString(doc, cur->children, 1));
         
     /** @todo Find a way to get rid of unit weight, it's redundant. */
      if ((!strcmp(cur->name, "Unitweight")) ) //&& (cur->ns == ns))
         bmat->wt = atof(xmlNodeListGetString(doc, cur->children, 1));

      if ((!strcmp(cur->name, "Youngsmod")) ) //&& (cur->ns == ns))
         bmat->ymod = atof(xmlNodeListGetString(doc, cur->children, 1));

      if ((!strcmp(cur->name, "Poissonratio")) ) //&& (cur->ns == ns))
         bmat->pois = atof(xmlNodeListGetString(doc, cur->children, 1));

      if ((!strcmp(cur->name, "Damping")) ) //&& (cur->ns == ns)) 
      {
         bmat->damping = atof(xmlNodeListGetString(doc, cur->children, 1));
         gotdamping = 1;  //kludge
      }

     /* These will have three values each */
      if ((!strcmp(cur->name, "Istress")) ) //&& (cur->ns == ns))
      {
         tempstring = xmlNodeListGetString(doc, cur->children, 1);
         checkval = sscanf(tempstring,"%lf%lf%lf",&temp[0],&temp[1],&temp[2]);
         if (checkval == 3) {
            bmat->iss[0] = temp[0];
            bmat->iss[1] = temp[1];
            bmat->iss[2] = temp[2];
         } else { 
            ddaml_display_error("Wrong number of initial stress values");
         }  
      }

      if ((!strcmp(cur->name, "Istrain")) ) //&& (cur->ns == ns))
      {
         tempstring = xmlNodeListGetString(doc, cur->children, 1);
         checkval = sscanf(tempstring,"%lf%lf%lf",&temp[0],&temp[1],&temp[2]);
         if (checkval == 3) {
            bmat->ist[0] = temp[0];
            bmat->ist[1] = temp[1];
            bmat->ist[2] = temp[2];
         } else {
            ddaml_display_error("Wrong number of initial strain values");
         }     
      }

      if ((!strcmp(cur->name, "Ivelocity")) ) //&& (cur->ns == ns))
      {
         tempstring = xmlNodeListGetString(doc, cur->children, 1);
         checkval = sscanf(tempstring,"%lf%lf%lf",&temp[0],&temp[1],&temp[2]);
         if (checkval == 3) {
            bmat->ivel[0] = temp[0];
            bmat->ivel[1] = temp[1];
            bmat->ivel[2] = temp[2];
         } else {
            ddaml_display_error("Wrong number of initial velocity values");
         }
      }

      cur = cur->next;
   }

   if (!gotdamping) {
      ddaml_display_error("Damping tag not found in BlockMaterials");
      exit(0);
   }

   dl_insert_b(blockmatlist,(void*)bmat);

}  



// Just for reference...
/*
typedef struct _loadpoint {
   double x, y;
   int loadpointsize1;
   int loadpointsize2;  // = 3 
   double ** vals;
}LOADPOINT;
*/
void 
parseLoadpoints(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {

   int i;
   int numloadpoints = 0;
   int parsepoints = 0;
   int checkval;
   double temp[3] = {0.0};
   Loadpoint * loadpoint;
   char * timehiststring;
   char * recordstring;

   //ddaml_display_warning(cur->name);
   cur = cur->children;

   //fprintf(stderr,"Parsing load points\n");
   //ddaml_display_warning("Parsing load points.");
   //ddaml_display_warning(cur->name);

   while (cur != NULL) {

      if ((!strcmp(cur->name, "Loadpoint")) ) {

         // TODO: Consider moving this to a function to make the inner 
         // easier to read.
        /* Get the size of this thing from the attribute value: */
        /* FIXME: Segfaults if no attribute specified.  Either output
         * from the xml parser must be examined, or the call needs to 
         * checked for NULL return value.  To reproduce this error,
         * pick some analysis with loading points, and delete the 
         * `numpoints="n"' attribute.
         */
         numloadpoints = atoi(xmlGetProp(cur,"numpoints"));
         if (numloadpoints < 2) {
            ddaml_display_error("2 or more load points are required.");
         }

         loadpoint = loadpoint_new();
         loadpoint->loadpointsize1 = numloadpoints+1;
         loadpoint->loadpointsize2 = 3+1;
         loadpoint->vals = DoubMat2DGetMem(loadpoint->loadpointsize1,
                                           loadpoint->loadpointsize2);
        /* Slurp up the entire time history... */
         timehiststring = xmlNodeListGetString(doc, cur->children, 1);
        /* Now, lots of heinous parsing... */
         recordstring = strtok(timehiststring, ";");

        /* FIXME: Check the behavior of this with respect to the 
         * start of indexing value 0 or 1.
         */
         for (i=0; i<numloadpoints; i++) {

            checkval = sscanf(recordstring,"%lf%lf%lf",
                              &temp[0],&temp[1],&temp[2]);
            if (checkval == 3) {
               parsepoints++;
               loadpoint->vals[i][0] = temp[0];
               loadpoint->vals[i][1] = temp[1];
               loadpoint->vals[i][2] = temp[2];
               recordstring = strtok(NULL, ";");
            } else {
                ddaml_display_warning("Empty load point list.\n");
            }

         }  

         if (parsepoints != numloadpoints) {
            ddaml_display_error("Load point att and val inconsistent");
         }

         dl_insert_b(loadpointlist, loadpoint);
         parsepoints = 0;
         numloadpoints = 0;
      } else if ((!strcmp(cur->name, "text")) ) {
         // Uncomment the following to see how libxml handles text elements
         // such as tabs and spaces.
         //ddaml_display_warning("Text node.");
      } else {

        /* Big problems.  Something passed the validator that was
         * not supposed to pass.  Do Something About It!
         */         
         // TODO: Extract the file name and line number of ddaml file
         // so that the analyst can fix it.
         ddaml_display_warning("Unknown element in load point parsing."  
                             " Contact author.\n");
      }
      cur = cur->next;
   }
}  



/* The values parsed here will correspond to the 
 * entries 7,8,9 in the geometry structure bolt array
 */ 
void 
parseBoltproperties(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {

   Boltmat * boltmat;
   char * tempstring;
   int checkval;  
   double stiffness, strength, pretension;

   cur = cur->children;
            
   tempstring = xmlNodeListGetString(doc, cur, 1);

  /* Everything below should be moved into the rock bolt
   * component file, because none of it depends on any
   * of the xml processing code.
   */
  /* Grab a struct for bolt properties */
   boltmat = boltmat_new();

   checkval = sscanf(tempstring,"%lf%lf%lf",
                     &stiffness,&strength,&pretension);
   if (checkval == 3) {
      boltmat_set_props(boltmat,stiffness,strength,pretension);
   } else {  
      ddaml_display_error("Wrong number of bolt property values");   
   }

   dl_insert_b(boltmatlist,(void*)boltmat);
}  



static void 
parseWritevertices(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) 
{
  /* Need to grab a tempstring, then look for a code to write ALL vertices. 
   * Default is to only write vertices for blocks containing measured points.
   */
   char * temp;

   fprintf(stdout,"Parsing Writevertices...\n");
   
   temp = xmlGetProp(cur,"type");

   if (!strcmp("all",xmlGetProp(cur,"flag")) ) {
      adata->verticesflag = 1;
   }
   else { 
      adata->verticesflag = 0;
   } // end if


   fprintf(stdout,"verticesflag: %d\n\n", adata->verticesflag);

}  

static double **
DoubMat2DGetMem(int n, int m)
{
   int i;
   double **x;

   assert ( (m!=0) && (n!=0) );

   //x = (double **)malloc(sizeof(double *)*n);
   x = (double **)calloc(n,sizeof(double *));
   if (x == NULL)
      return NULL;

   for ( i = 0; i < n; ++i)
   {
      //x[i] = (double *)malloc(sizeof(double)*m);
      x[i] = (double *)calloc(m,sizeof(double));
      if(x[i] == NULL)
         return NULL;
      //else 
         //memset(x[i], 0xDDA, m);
   }

   return x;

}  



#if 0
static JointMat *
getNewJointMat(void)
{
   JointMat * jm;
   //fprintf(stdout,"Getting new joint\n");
   jm = (JointMat *)calloc(1,sizeof(JointMat));
   return jm;

}  /*  close getNewJointMat() */
#endif


static BlockMat *
getNewBlockMat(void)
{
   BlockMat * bm;
   //fprintf(stdout,"Getting new block mat\n");
   bm = (BlockMat *)malloc(sizeof(BlockMat));
   memset(bm,0xDA,sizeof(BlockMat));
   return bm;

}  /*  close getNewJointMat() */






KWDTAB atab_type[] = {
{"Writevertices",  0, *parseWritevertices  },
{"Rotation",       0, *parseRotation       },
{"Gravity",        0, *parseGravity        },
{"Autotimestep",   0, *parseAutotimestep   },
{"Autopenalty",    0, *parseAutopenalty    },
{"Analysistype",   0, *parseAnalysistype   },
{"Numtimesteps",   0, *parseNumtimesteps   },
{"Planestrain",    0, *parsePlanestrain    },
//{"Analysis",     0, *parseAnalysis       },
{"Jointproperties",0, *parseJointproperties},
{"Loadpointlist",  0, *parseLoadpoints     },
{"Blockmaterial",  0, *parseBlockmaterial  },
{"Boltproperties", 0, *parseBoltproperties },
{"AConstants",     0, *parseAConstants     },
{"Opencloselimit" ,0, *parseOpencloselimit },
{"Saveinterval",   0, *parseSaveinterval   },
{"Maxtimestep",    0, *parseMaxtimestep    }, 
{"Mintimestep",    0, *parseMintimestep    }, 
{"Maxdisplacement",0, *parseMaxdisplacement},
{"Timehistory",    0, *parseTimehistory    },
{"ContactDamping", 0, *parseContactDamping },
{"Gravaccel",      0, *parseGravaccel      },
{NULL,             0,  0                   }			
};


void
parseAnalysis(Analysisdata * ad, xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {

   int i = 0;

  /* Easiest to parse into a list, then count 
   * and grab array memory, then copy data into 
   * arrays.  This will be good, because I will 
   * now have lists of elements that can eventually
   * be used in the main code.
   */
   initializeALists(); 

   while (cur != NULL) 
   {
      i = 0;
      while (atab_type[i].kwd)
      {
         if ((!strcmp(atab_type[i].kwd,cur->name)) ) //&& (cur->ns == ns)) 
         {
            fprintf(stderr,"A Keyscan loop: Current name %s:\n", cur->name);
            switch(atab_type[i].ktok)
            {
               case node:
	                 atab_type[i].parsefn.nodeparse(doc,ns,cur);
               break;

               case string:
	                 atab_type[i].parsefn.stringparse(doc,cur,1);
               break;

               case prop:
	                 atab_type[i].parsefn.propparse(doc,cur,1);
               break;

               default:;
               break;
            }  /* end switch on node type */
         }
         i++;
      }   //close key scan loop 

      cur = cur->next;

   }  /* end loop over current pointer */

  /** Transfer data from list format to array format. */
   transferAData();

  /* @warning  Gravity kludge, needs to be fixed. 
   * A better way to do this would be to check to see
   * whether the gravity tag and attribute are seen
   * and set properly.
   */
   if (adata->gravaccel == -1) {
      adata->gravaccel = 9.81;
   }
}  


void
ddaml_read_analysis_file(Analysisdata * ad, char *filename) {

   xmlDocPtr doc;
   xmlNsPtr ns;
   xmlNodePtr cur;

   xmlNode *root_element = NULL;


   //xmlDoValidityCheckingDefaultValue = 1;

   adata = ad;

   ddaml_display_error   = ad->display_error;

  /**
   * build an XML tree from the file;
   */
   doc = xmlParseFile(filename);


   //ddaml_check_document((void*)doc,"http://www.tsoft.com/~bdoolin/dda","DDA");
   //cur = doc->root;

   root_element = xmlDocGetRootElement(doc);
   cur = root_element;
   ns = nspace;

  /** Now, walk the tree.  All the following code will
   * be replaced with keyscanning.  Initially, do not
   * check for keyscanning failure.  Let the dtd validator
   * handle that.  Later, a few simple checks can be added,
   * just to handle exceptional cases.
   */
  /* FIXME:  This needs to be handled as an exception to
   * protect against dereferencing a null pointer.
   */
   cur = cur->children;

   while (cur != NULL) {
         if ( !strcmp( cur->name, "Analysis") )  {
            cur = cur->children;
            parseAnalysis(ad,doc, ns, cur); 
            break;
         }
	     cur = cur->next;
   }

   assert(ad != NULL);
} 




#ifdef STANDALONE
int 
main(int argc, char **argv) 
{
   int i;
   fprintf(stderr,"Parsing analysis data...\n");

   for (i = 1; i < argc ; i++) {
	     adata = XMLparseDDA_Analysis_File(argv[i]);
   }

   return(0);
}  

#endif /* STANDALONE */