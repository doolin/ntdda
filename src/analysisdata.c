

#ifdef WIN32
#pragma warning( disable : 4115 )        
#endif

#include <string.h>
#include <math.h>
#include <assert.h>

#include "analysisdata.h"
#include "analysis.h"
#include "ddamemory.h"
#include "gravity.h"
#include "ddaml.h"


//extern InterFace * iface;

#define I1 "   "
#define I2 "      "


/** FIXME: Get rid of these prototypes. */
static void dumpDDAMLAnalysisFile(Analysisdata *, FILE * outfilestream);
static void emitAConstants(CONSTANTS *, FILE *);
static void emitBlockMaterials(Analysisdata *, FILE *);
static void emitJointMaterials(Analysisdata *, FILE *);




static void
abortAnalysis(Analysisdata * ad) {

  /* Save the time step where the analysis was aborted.  This will 
   * be needed for writing data after the analysis ends.
   */
   ad->abortedtimestep = ad->currTimeStep;
  /* When flow enters the top of the main analysis loop,
   * it will end directly, and all the normal stuff that happens
   * afterward, such as freeing memory and writing output files
   * happens automatically.
   */
   ad->currTimeStep = ad->nTimeSteps;
  /* Reset so the function that writes the data to the 
   * log file does not write out a bunch of unnecessary 
   * zero values.
   */
   ad->nTimeSteps = ad->abortedtimestep;

  /* FIXME: Here is a good place to disable some menu 
   * items.
   */
  return;

}  /* close abortAnalysis() */



/* FIXME: Change name to ddaml_analysis_write_file() */
static void
dumpDDAMLAnalysisFile(Analysisdata * ad, FILE * outfile)
{
   //int i;
   //FILE * outfile;
  /* FIXME: Move the sizing constant to somewhere else.
   */
   char attribute[180];

  /* FIXME: Return an error if this fails. */
   //outfile = fopen(outfilename,"w");

  /* xml header */
   fprintf(outfile,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
   fprintf(outfile,"<!DOCTYPE DDA SYSTEM \"analysis.dtd\">\n");
   fprintf(outfile,"<Berkeley:DDA xmlns:Berkeley=\"http://www.tsoft.com/~bdoolin/dda\">\n");

  /* FIXME:  This is a bogosity.  The parser code is broken
   * because it requires a comment before the <Geometry>
   * or <Analysis> tags.
   */
   fprintf(outfile,"<!-- Bogus comment to keep ddaml tree-stripping\n");
   fprintf(outfile,"     from seg faulting on bad child node. -->\n\n");


   if (ad->analysistype == 0)
      strcpy(attribute,"static");
   else 
      strcpy(attribute,"dynamic");
   fprintf(outfile,"<Analysis type=\"%s\">\n\n",attribute);
   fprintf(outfile,I1"<Analysistype type=\"%s\"/>\n",attribute);

   switch (ad->rotationtype)
   {
      case linear:
         strcpy(attribute,"linear");
         break;
      case secondorder:
         strcpy(attribute,"secondorder");
         break;
      case exact:
         strcpy(attribute,"exact");
         break;
      default:
        /* FIXME: Throw an error here. */
         break;
   }
   fprintf(outfile,I1"<Rotation type=\"%s\"/>\n",attribute);

   
   if (ad->gravityflag == 0)
      strcpy(attribute,"no");
   else 
      strcpy(attribute,"yes");
   fprintf(outfile,I1"<Gravity flag=\"%s\">\n",attribute);
   fprintf(outfile,I1"Gravity parameters are currently hardwired.\n");
   fprintf(outfile,I1"</Gravity>\n\n");

   if (ad->autotimestepflag == 0)
      strcpy(attribute,"no");
   else 
      strcpy(attribute,"yes");
   fprintf(outfile,I1"<Autotimestep flag=\"%s\"/>\n",attribute);

   if (ad->autopenaltyflag == 0)
      strcpy(attribute,"no");
   else 
      strcpy(attribute,"yes");
   fprintf(outfile,I1"<Autopenalty flag=\"%s\"/>\n",attribute);


   if (ad->planestrainflag == 0)
      strcpy(attribute,"no");
   else 
      strcpy(attribute,"yes");
   fprintf(outfile,I1"<Planestrain flag=\"%s\"/>\n",attribute);
 
   
   fprintf(outfile,I1"<Numtimesteps timesteps=\"%d\"/>\n",ad->nTimeSteps);
   fprintf(outfile,I1"<Maxtimestep maxtimestep=\"%f\"/>\n",ad->maxtimestep);
   fprintf(outfile,I1"<OCLimit maxopenclose=\"%d\"/>\n",ad->OCLimit);
   fprintf(outfile,I1"<Maxdisplacement maxdisplacement=\"%f\"/>\n",ad->maxdisplacement);
   fprintf(outfile,I1"<Saveinterval step=\"%d\"/>\n",ad->tsSaveInterval);


   fprintf(outfile,I1"<!-- These used to be hard-wired into the\n"); 
   fprintf(outfile,I1"     DDA source code.  Not currently used -->\n");

   emitAConstants(ad->constants, outfile);

   //emitLoadpoints(ad, outfile);

   emitBlockMaterials(ad, outfile);

   emitJointMaterials(ad, outfile);

   fprintf(outfile,"</Analysis>\n");
   fprintf(outfile,"</Berkeley:DDA>\n");

   fclose(outfile);

}  /* close dumpDDAMLGeometryFile() */


static void 
emitAConstants(CONSTANTS * constants, FILE * outfile) {

   //CONSTANTS * constants = ad->constants;


  /* FIXME:  Actually use the constants instead of the 
   * hardwired default values here.
   */
   fprintf(outfile,I1"<AConstants>\n");
   //fprintf(outfile,I2"<Openclose value=\"0.0002\"/>\n");
   fprintf(outfile,I2"<Openclose value=\"%f\"/>\n",constants->openclose );
   fprintf(outfile,I2"<Opencriteria value=\"0.0000002\"/>\n");
   fprintf(outfile,I2"<Domainscale value=\"0.0004\"/>\n");
   fprintf(outfile,I2"<NormSpringPen value=\"derived\"/>\n");
   fprintf(outfile,I2"<NormExternDist value=\"derived\"/>\n");
   fprintf(outfile,I2"<NormPenDist value=\"derived\"/>\n");
   fprintf(outfile,I2"<AngleOverlap value=\"3.0\"/>\n");
   fprintf(outfile,I2"<ShearNormRatio value=\"2.5\"/>\n");
   fprintf(outfile,I1"</AConstants>\n\n");

}  /* close emitAConstants() */


/** FIXME: Implement this function. */
/*
static void 
emitLoadpoints(Analysisdata * ad, FILE * outfile) {
}  
*/

static void 
emitBlockMaterials(Analysisdata * ad, FILE * outfile)
{
   int i;

  /* FIXME: Use a predefined SIZE value here
   * along with strncy or strlcpy for safety.
   */
   char stress[180];
   char strain[180];
   char velocity[180];

   for (i=1; i<= ad->nBlockMats; i++)
   {
      fprintf(outfile,I1"<Blockmaterial  type=\"%d\">\n",i);
   
      fprintf(outfile,I2"<Unitmass> %f </Unitmass>\n",ad->materialProps[i][0]);
      fprintf(outfile,I2"<Unitweight> %f </Unitweight>\n",ad->materialProps[i][1]);
      fprintf(outfile,I2"<Youngsmod> %f </Youngsmod>\n",ad->materialProps[i][2]);
      fprintf(outfile,I2"<Poissonratio> %f </Poissonratio>\n",ad->materialProps[i][3]);

      sprintf(stress, "%f  %f  %f", ad->materialProps[i][4],
                           ad->materialProps[i][5],ad->materialProps[i][6]);
      fprintf(outfile,I2"<Istress> %s </Istress>\n", stress);

      sprintf(velocity, "%f  %f  %f", ad->materialProps[i][10],
                           ad->materialProps[i][11],ad->materialProps[i][12]);
      fprintf(outfile,I2"<Ivelocity> %s </Ivelocity>\n", velocity);

      sprintf(strain, "%f  %f  %f", ad->materialProps[i][7],
                           ad->materialProps[i][8],ad->materialProps[i][9]);
      fprintf(outfile,I2"<Istrain> %s </Istrain>\n", strain);

      fprintf(outfile,I1"</Blockmaterial>\n");


   }  /*  i  */


}  /* close emitBlockMaterials() */



static void 
emitJointMaterials(Analysisdata * ad, FILE * outfile)
{
   int i;

   for (i=1; i<= ad->nJointMats; i++)
   {  
      fprintf(outfile,I1"<Jointproperties type=\"%d\">\n",i);  
      fprintf(outfile,I2"<Friction> %.2f</Friction>\n",ad->phiCohesion[i][0]);
      fprintf(outfile,I2"<Cohesion> %.2f</Cohesion>\n",ad->phiCohesion[i][1]);
      fprintf(outfile,I2"<Tensile> %.2f</Tensile>\n",ad->phiCohesion[i][2]);
      fprintf(outfile,I1"</Jointproperties>\n");  
   }  /*  i  */

}  /* close emitJointMaterials() */



/*
static void
emitBoltMaterials(Analysisdata * ad, FILE * outfile) {
}  
*/



/* TODO: Instead of exit(0), construct an API for 
 * "fixing" mistakes, and use that instead.
 */
void
adata_validate(Analysisdata * ad) {

   int i;
   int nb = ad->nBlockMats;
   //double grav = ad->gravaccel; 
   //double ** materialProps = ad->materialProps;

  /* Check to make sure that density and unit weight
   * are related by the value of gravity.
   */
   for (i=1; i<= nb; i++) {
     /* That is, set the initial spring stiffness
      * equal to the highest Young's modulus...
      */
      /* This code should no longer be necessary since 
       * the unit weight is computed from the density 
       * and gravitational acceleration.
       */
      /*
      if (fabs((grav*materialProps[i][0] - materialProps[i][1])) > 0.01) 
      {
         iface->displaymessage("Density and unit weight are inconsistent");
         //exit(0);
      }
      */
   }  /*  i  */


  /* If we have a time history, we need to make sure the time
   * interval in the time series matches the analysis time 
   * step.
   */
   if (ad->timehistory != NULL) {
      if (th_get_delta_t(ad->timehistory) != ad->delta_t) {

         //iface->displaymessage("Analysis time step must be equal to time interval in time history.");
         ad->display_error("Analysis time step must be equal to time interval in time history.");

         exit(0);
      }
   }

}  /* close validateAnalysisdata() */



/* Attempt to handle extern file pointer
 * declarations as a group.  That is, open them 
 * all at the same time, then later close all of the 
 * at the same time.
 * FIXME: replace all strcpy with strncpy for buffer
 * protection when the back end goes networked.
 * FIXME: The code for initializing file names should be split
 * from the code that actually opens the output streams.
 * This will make it easier to control which files should 
 * actually be open at any one time.
 */
void
openAnalysisFiles(FILEPATHS * filepath)
{
   char temp[256];
   extern FILEPOINTERS fp;

	 	strcpy(temp, filepath->gfile);

  /* Note that the rootname contains the entire path.  This 
   * is probably not real good, but is messy to handle 
   * otherwise on win32.
   */
   strcpy(temp, filepath->rootname);
   strcat(temp, ".replay");
   strcpy(filepath->replayfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".html");
   strcpy(filepath->htmlfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_data.m");
   strcpy(filepath->datafile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".gnu");
   strcpy(filepath->gnuplotfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".log");
   strcpy(filepath->logfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_meas.m");
   strcpy(filepath->measfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".por");
   strcpy(filepath->porefile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_time.m");
   strcpy(filepath->timefile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".par");
   strcpy(filepath->parfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_moments.m");
   strcpy(filepath->momentfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_mass.m");
   strcpy(filepath->massfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_defs.m");
   strcpy(filepath->dfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".grav");
   strcpy(filepath->gravfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_fpoint.m");
   strcpy(filepath->fpointfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_cforce.m");
   strcpy(filepath->cforce, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_fforce.m");
   strcpy(filepath->fforce, temp);

  /* Copied in from geometry driver. */
   strcpy(temp, filepath->rootname);
   strcat(temp, ".blk");
   strcpy(filepath->blockfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".err");
   strcpy(filepath->errorfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".spy1");
   strcpy(filepath->spyfile1, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".spy2");
   strcpy(filepath->spyfile2, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, ".m");
   strcpy(filepath->mfile, temp);

   strcpy(temp, filepath->rootname);
   strcat(temp, "_stress.m");
   strcpy(filepath->stressfile, temp);

  /* Do not get the file name of this yet. */
   //strcpy(temp, filepath->gpath);
   //strcat(temp, ".m");
   strcpy(filepath->eqfile, filepath->gpath);

   fp.replayfile = fopen(filepath->replayfile, "w");
   fp.measfile = fopen(filepath->measfile, "w");
   fp.logfile = fopen(filepath->logfile, "w");
   fp.errorfile = fopen(filepath->errorfile, "w");
  /* What is dispfile?
   */
   //fp.dispfile = fopen(filepath->dispfile, "w");
   //fp.grffile = fopen(filepath->grffile, "w");
  /* Cleft pressure file output. */
   fp.porefile = fopen(filepath->porefile, "w");
  /* time data */
   fp.timefile = fopen(filepath->timefile, "w");
  /* parameter file */
   fp.parfile = fopen(filepath->parfile, "w");
  /* block area file */
   fp.momentfile = fopen(filepath->momentfile, "w");
   fp.massfile = fopen(filepath->massfile, "w");
   fp.gravfile = fopen(filepath->gravfile, "w");
   fp.htmlfile = fopen(filepath->htmlfile, "w");
   fp.datafile = fopen(filepath->datafile, "w");
   fp.fpointfile = fopen(filepath->fpointfile, "w");

   fp.cforce = fopen(filepath->cforce, "w");
   fprintf(fp.cforce,"contactforces = [\n");

   fp.fforce = fopen(filepath->fforce, "w");
   fprintf(fp.fforce,"frictionforces = [\n");


   
   fp.spyfile1 = fopen(filepath->spyfile1, "w");
   fp.spyfile2 = fopen(filepath->spyfile2, "w");
   fp.gnuplotfile = fopen(filepath->gnuplotfile, "w");
   fp.mfile = fopen(filepath->mfile, "w");


   fp.dfile = fopen(filepath->dfile, "w");
   fprintf(fp.dfile,"deformations = [\n");

   fp.stressfile = fopen(filepath->stressfile, "w");
   fprintf(fp.stressfile,"stresses = [\n");

  /* Do not open this yet.  The function that parses this
   * data should handle all the io for the time history.
   */
  /*fp.eqfile = fopen(filepath->eqfile,"r");*/

} /* Close openAnalysisFiles() */


void
closeAnalysisFiles()
{
   extern FILEPOINTERS fp;

   fclose(fp.replayfile);
   //fprintf(fp.logfile, "Closed log file.\n");
   fclose(fp.logfile);
   //fprintf(fp.errorfile, "Closed error file.\n");
   fclose(fp.errorfile);
   //fclose(fp.dispfile);
   //fclose(fp.grffile);
   fclose(fp.measfile);
   fclose(fp.porefile);
   fclose(fp.timefile);
   fclose(fp.parfile);
   fclose(fp.momentfile);
   fclose(fp.massfile);
   fclose(fp.gravfile);
   fclose(fp.htmlfile);
   fclose(fp.datafile);
   fclose(fp.fpointfile);

   fprintf(fp.cforce,"];\n");
   fclose(fp.cforce);

   fprintf(fp.fforce,"];\n");
   fclose(fp.fforce);

   fclose(fp.spyfile1);
   fclose(fp.spyfile2);
   fclose(fp.gnuplotfile);
   fclose(fp.mfile);

   fprintf(fp.dfile,"];\n");
   fclose(fp.dfile);


  /* FIXME: This is non-portable. */
   fprintf(fp.stressfile,"];\n");
   fclose(fp.stressfile);

  /* Cosed in function handling the time history.
   * Noted for completeness and uniformity.
   */
   //fclose(fp.eqfile);

}  /* Close openAnalysisFiles() */





/*
void 
adata_delete(Analysisdata * ad)
{
   freeAnalysisData(ad);
}
*/

void *
adata_delete(Analysisdata * ad) {

   free2DMat((void **)ad->timeDeps, ad->timedepsize1);
   free2DMat((void **)ad->materialProps, ad->materialpropsize1);
   free2DMat((void **)ad->phiCohesion, ad->phicohesionsize1);
   free2DMat((void **)ad->globalTime, ad->globaltimesize1);
   free2DMat((void **)ad->K, ad->ksize1);
   free2DMat((void **)ad->Kcopy, ad->ksize1);
   free2DMat((void **)ad->F,ad->Fsize1);
   free2DMat((void **)ad->Fcopy,ad->Fsize1);
   free2DMat((void **)ad->tindex, ad->k5size1);
   free2DMat((void **)ad->c, ad->csize1);
   if (ad->gravity)
      destroyGravity(ad->gravity);
   free(ad->springstiffness);
   free(ad->avgArea);
   if (ad->loadpoints)
      freeLoadpoints(ad->loadpoints);
   if(ad->constants)
      free(ad->constants);
   if (ad->initialconstants)
      free(ad->initialconstants);

   free(ad);

   return NULL;
}  /* Close freeAnalysisData() */



void
adata_read_input_file(Analysisdata * ad, char * infilename, int numfixedpoints,
                      int pointcount, int numloadpoints) {

   IFT afv;

   afv = getFileType(infilename);

   switch(afv)
   {
       case ddaml:
          ddaml_read_analysis_file((void*)ad,infilename);
          break;
          
       case extended: 
          //iface->displaymessage("Extended analysis file format deprecated");
          //AData->printer(NULL,"Warning","Extended analysis file format deprecated");
          //exit(0);
         /*
          AData = analysisReader2(analysisFile, gd->pointCount);
          if (!AData)
          {
             iface->displaymessage("Error in analysis file");
             return NULL;
          }
          break;
          */
       case original:
       default:          
          analysisReader1(ad, infilename, numfixedpoints,pointcount,numloadpoints);  
          ad->display_warning("Obsolete geometry file detected");
          break;
   }
}  



void 
freeLoadpoints(LOADPOINT * lp) {

   if (lp->vals)
      free2DMat((void**)lp->vals,lp->loadpointsize1);
   free(lp);
}  /* close freeLoadPoints() */



void 
adata_set_output_flag(Analysisdata * ad, int flag)
{
   ad->options |= flag;
}

void 
adata_clear_output_flag(Analysisdata * ad, int flag)
{
   ad->options ^= flag;
}

/* 
 * Is mathematical analysis...only a vain play of the mind?
 * It can give to the physicist only a convenient language;
 * is this not a mediocre service, which, strictly speaking,
 * could be done without; and even is it not to be feared
 * that this artificial language may be a veil interposed
 * between reality and the eye of the physicist?  Far from 
 * it; without this language most of the intimate analogies
 * of things would heave remained forever unknown to us;
 * and we should forever have been ignorant of the internal 
 * harmony of the world, which is...the only true reality.
 *     --- Henri Poincar\'e
 */


/* FIXME: Pull the relevant comments out of here and ensure
 * they are in the correct header files.
 */
Analysisdata *
adata_clone(Analysisdata * adn) {

   int i;

   Analysisdata * ado;  /* i.e, ad out */
   
   /* make a call to the initAnalysis data here instead */
   ado = (Analysisdata *)malloc(sizeof(Analysisdata));

   memcpy((void*)ado,(void*)adn,sizeof(Analysisdata));

   ado->tindex = clone2DMatInt(adn->tindex,adn->k5size1,adn->k5size2);
   ado->c = clone2DMatDoub(adn->c,adn->csize1,adn->csize2);
   ado->timeDeps = clone2DMatDoub(adn->timeDeps,adn->timedepsize1,adn->timedepsize2);
   ado->materialProps = clone2DMatDoub(adn->materialProps,adn->materialpropsize1,adn->materialpropsize2);
   ado->phiCohesion = clone2DMatDoub(adn->phiCohesion, adn->phicohesionsize1,adn->phicohesionsize2);
   ado->globalTime = clone2DMatDoub(adn->globalTime, adn->globaltimesize1,adn->globaltimesize2);
  /* global stiffness matrix, was `a' */
   ado->K = clone2DMatDoub(adn->K, adn->ksize1, adn->ksize2);
  /* copy of K from previous step */
   ado->Kcopy = clone2DMatDoub(adn->Kcopy, adn->ksize1, adn->ksize2);

   for (i=0; i<=adn->springstiffsize; i++)
   {
      ado->springstiffness[i] = adn->springstiffness[i];
   }
   for (i=1; i<=adn->avgareasize; i++)
   {
      ado->avgArea[i] = adn->avgArea[i];
   }

   ado->initialconstants = cloneConstants(adn->initialconstants);
   
  /* FIXME: clone the timehistory points/array also: */
   //ado->timehistory = cloneTimeHistory(adn->timehistory);

   return ado;

}  /* Close cloneAnalysisData() */


Analysisdata *
adata_new() {
  /* According to Kelley and Pohl, the code in 
   * NEWINIT should perform the same initialization
   * functions as the code in OLDINIT.  However,
   * what the OLDINIT code does is suck up all the ram
   * and make NT die.  FIXME: find out whether the 
   * initialization problem is an MS problem, or a
   * non-standard c construct.
   */

   Analysisdata * ado;

  /* FIXME:  change to malloc. */
   ado = (Analysisdata *)calloc(1,sizeof(Analysisdata));
   
  /* FIXME: This makes everything blow up. Probably there is behavior
   * that is relying on NULL pointers (e.g., timehistory below)
   * so giving this a default value ensures a segfault.
   */
   //memset(ado, 0xDA, sizeof(Analysisdata));


   /* Doing this basically introduces a user interface bug.
    * We need something to note whether the gravity tag is 
    * seen by the xml parser.  Then this can go away.
    */
   ado->gravaccel = -1;

   ado->options = 0;

   ado->tsSaveInterval = 5;

  /* Constants are now malloced in their own function */
   ado->constants = NULL; //(CONSTANTS *)calloc(1,sizeof(CONSTANTS));
   ado->initialconstants = NULL; // (CONSTANTS *)calloc(1,sizeof(CONSTANTS));

  /* NULL flag is useful. */
   ado->timehistory = NULL;

/*****  Options ***********/
  /* These need to be options put into the DDAML DTD */
  /* These options moved in here from compilecontrol.c */
   //AData->seismicflag = 0; //FALSE;
   ado->solvetype = lu;
  /* Not really implemented yet...  Next step is to set the
   * initialize the lagrange variables before the open-close
   * iteration starts for any particular time step.
   */
   //AData->contactmethod = auglagrange; 
   ado->contactmethod = penalty; 
   ado->integrator = constant;
  /* TODO: Set this from the xml file at least. */
   ado->tsSaveInterval = 1;
   ado->frictionlaw = tpmc;

  /* Desparate kludge */
   ado->writemfile = TRUE;

/**************************/

  /* FIXME: Set up the private functions.  These can be moved in the
   * future to the analysis initialization function.
   */
   ado->abort = abortAnalysis;
   ado->deletematerial = deleteBlockMaterial;

  /* Functions */
   ado->dump = dumpDDAMLAnalysisFile;
   ado->free = adata_delete; //freeAnalysisData;


  /* FIXME: The xml input file has this as a user controlled 
   * parameter.  8 is default value from GHS code.
   */
   if (ado->OCLimit == 0)
      ado->OCLimit = 6+2;

   return ado;
}  /* Close initAnalysisData() */


void
adata_set_contact_damping(Analysisdata * ad, double d) {
   ad->contact_damping = d;
}

double
adata_get_contact_damping(Analysisdata * ad) {
   return ad->contact_damping;
}

void 
adata_set_grav_accel(Analysisdata * ad, double grav) {
   ad->gravaccel = grav;
}

double 
adata_get_grav_accel(Analysisdata * ad){
   return ad->gravaccel;
}