

#ifdef WIN32
#pragma warning( disable : 4115 )        
#endif


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>


#include "analysisdata.h"
#include "ddamemory.h"
#include "ddafile.h"
#include "ddaml.h"


//#include "analysis.h"


#define I1 "   "
#define I2 "      "



static void emitBlockMaterials(Analysisdata *, PrintFunc printer, void * stream);
static void emitJointMaterials(Analysisdata *, PrintFunc printer, void * stream);

static void deleteBlockMaterial(Analysisdata * ad, int blocknumber);


/*
 * WARNING!!!  This function is invoked through an analysis struct
 * function pointer that is set when the analysis data is initialized.
 * It should only be called in tandem with deleteBlock.
 */
static void
deleteBlockMaterial(Analysisdata * ad, int blocknumber) {

   int i,j,k;
   int mpsize1 = ad->materialpropsize1;  //  nBlocks
   int mpsize2 = ad->materialpropsize2;  //  indexing from 0
   double ** e0 = ad->materialProps;

   j = 0;
   for (i=1;i<=mpsize1;i++)    {

       if (i == blocknumber) {

          j++;
          ad->materialpropsize1--;
          continue;
       }

       for (k=0;k<=mpsize2;k++) {

          e0[i-j][k] = e0[i][k];
       }
    }

   /* Set the memory in what was the last element to garbage to make
    * sure it isn't accidently used.
    */ 
    memset((void*)e0[mpsize1],0xDA,mpsize2*sizeof(double));
}  




static void
abortAnalysis(Analysisdata * ad) {

  /* Save the time step where the analysis was aborted.  This will 
   * be needed for writing data after the analysis ends.
   */
   ad->abortedtimestep = ad->cts;
  /* When flow enters the top of the main analysis loop,
   * it will end directly, and all the normal stuff that happens
   * afterward, such as freeing memory and writing output files
   * happens automatically.
   */
   ad->cts = ad->nTimeSteps;
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



void
adata_write_ddaml(Analysisdata * ad, PrintFunc printer, void * outfile) {




  /* FIXME: Move the sizing constant to somewhere else.
   */
   char attribute[180];


  /* xml header */
   printer(outfile,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
   printer(outfile,"<!DOCTYPE DDA SYSTEM \"analysis.dtd\">\n");
   printer(outfile,"<Berkeley:DDA xmlns:Berkeley=\"http://www.tsoft.com/~bdoolin/dda\">\n");

  /* FIXME:  This is a bogosity.  The parser code is broken
   * because it requires a comment before the <Geometry>
   * or <Analysis> tags.
   */
   printer(outfile,"<!-- Bogus comment to keep ddaml tree-stripping\n");
   printer(outfile,"     from seg faulting on bad child node. -->\n\n");


   if (ad->analysistype == 0)
      strcpy(attribute,"static");
   else 
      strcpy(attribute,"dynamic");

   printer(outfile,"<Analysis type=\"%s\">\n\n",attribute);
   printer(outfile,I1"<Analysistype type=\"%s\"/>\n",attribute);

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
   printer(outfile,I1"<Rotation type=\"%s\"/>\n",attribute);

   
   if (ad->gravityflag == 0)
      strcpy(attribute,"no");
   else 
      strcpy(attribute,"yes");

   printer(outfile,I1"<Gravity flag=\"%s\">\n",attribute);
   printer(outfile,I1"Gravity parameters are currently hardwired.\n");
   printer(outfile,I1"</Gravity>\n\n");

   if (ad->autotimestepflag == 0)
      strcpy(attribute,"no");
   else 
      strcpy(attribute,"yes");
   printer(outfile,I1"<Autotimestep flag=\"%s\"/>\n",attribute);

   if (ad->autopenaltyflag == 0)
      strcpy(attribute,"no");
   else 
      strcpy(attribute,"yes");
   printer(outfile,I1"<Autopenalty flag=\"%s\" pfactor=\"50\"/>\n",attribute);


   if (ad->planestrainflag == 0)
      strcpy(attribute,"no");
   else 
      strcpy(attribute,"yes");
   printer(outfile,I1"<Planestrain flag=\"%s\"/>\n",attribute);
 
   
   printer(outfile,I1"<Numtimesteps timesteps=\"%d\"/>\n",ad->nTimeSteps);
   printer(outfile,I1"<Maxtimestep maxtimestep=\"%f\"/>\n",ad->maxtimestep);
   printer(outfile,I1"<OCLimit maxopenclose=\"%d\"/>\n",ad->OCLimit);
   printer(outfile,I1"<Maxdisplacement maxdisplacement=\"%f\"/>\n",ad->maxdisplacement);
   printer(outfile,I1"<Saveinterval step=\"%d\"/>\n",ad->tsSaveInterval);


   printer(outfile,I1"<!-- These used to be hard-wired into the\n"); 
   printer(outfile,I1"     DDA source code.  Not currently used -->\n");

   constants_print_xml(ad->constants, printer, outfile);

/** 
 * @todo implement loadpoints_print_xml
 */
   //loadpoints_print_xml(ad, outfile);

   emitBlockMaterials(ad, printer, outfile);

   emitJointMaterials(ad, printer, outfile);

   printer(outfile,"</Analysis>\n");
   printer(outfile,"</Berkeley:DDA>\n");

}  



static void 
emitBlockMaterials(Analysisdata * ad, PrintFunc printer, void * outfile)
{
   int i;

  /* FIXME: Use a predefined SIZE value here
   * along with strncy or strlcpy for safety.
   */
   char stress[180];
   char strain[180];
   char velocity[180];

   for (i=1; i<= ad->nBlockMats; i++) {

      printer(outfile,I1"<Blockmaterial  type=\"%d\">\n",i);
   
      printer(outfile,I2"<Unitmass> %f </Unitmass>\n",ad->materialProps[i][0]);
      printer(outfile,I2"<Unitweight> %f </Unitweight>\n",ad->materialProps[i][1]);
      printer(outfile,I2"<Youngsmod> %f </Youngsmod>\n",ad->materialProps[i][2]);
      printer(outfile,I2"<Poissonratio> %f </Poissonratio>\n",ad->materialProps[i][3]);
      printer(outfile,I2"<Damping> %f </Damping>\n",ad->materialProps[i][13]);


      sprintf(stress, "%f  %f  %f", ad->materialProps[i][4],
                           ad->materialProps[i][5],ad->materialProps[i][6]);
      printer(outfile,I2"<Istress> %s </Istress>\n", stress);

      sprintf(velocity, "%f  %f  %f", ad->materialProps[i][10],
                           ad->materialProps[i][11],ad->materialProps[i][12]);
      printer(outfile,I2"<Ivelocity> %s </Ivelocity>\n", velocity);

      sprintf(strain, "%f  %f  %f", ad->materialProps[i][7],
                           ad->materialProps[i][8],ad->materialProps[i][9]);
      printer(outfile,I2"<Istrain> %s </Istrain>\n", strain);

      printer(outfile,I1"</Blockmaterial>\n");


   }  


}  /* close emitBlockMaterials() */



static void 
emitJointMaterials(Analysisdata * ad, PrintFunc printer, void * outfile) {


   int i;

   for (i=1; i<= ad->nJointMats; i++) {

     
      printer(outfile,I1"<Jointproperties type=\"%d\">\n",i);  
      printer(outfile,I2"<Friction> %.2f</Friction>\n",ad->phiCohesion[i][0]);
      printer(outfile,I2"<Cohesion> %.2f</Cohesion>\n",ad->phiCohesion[i][1]);
      printer(outfile,I2"<Tensile> %.2f</Tensile>\n",ad->phiCohesion[i][2]);
      printer(outfile,I1"</Jointproperties>\n");  
   } 

}  



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
   }  

}  





void 
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

   /*
   if (ad->gravity)
      destroyGravity(ad->gravity);
   */

   free(ad->springstiffness);
   free(ad->avgArea);
   loadpoint_delete(ad->loadpoints);
   if(ad->constants)
      free(ad->constants);
   if (ad->initialconstants)
      free(ad->initialconstants);

   free(ad);

}



void
adata_read_input_file(Analysisdata * ad, char * infilename, int numfixedpoints,
                      int pointcount, int numloadpoints) {

   IFT afv;

   afv = ddafile_get_type(infilename);

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
          ddafile_read_original_analysis(ad, infilename, numfixedpoints,pointcount,numloadpoints);  
          ad->display_warning("Obsolete geometry file detected");
          break;
   }
}  




void 
adata_set_output_flag(Analysisdata * ad, int flag) {
   ad->options |= flag;
}

void 
adata_clear_output_flag(Analysisdata * ad, int flag) {
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

   ado->initialconstants = constants_clone(adn->initialconstants);
   
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
   ado->constants = constants_new_defaults(); //NULL; //(CONSTANTS *)calloc(1,sizeof(CONSTANTS));
   ado->initialconstants = NULL; // (CONSTANTS *)calloc(1,sizeof(CONSTANTS));

  /* NULL flag is useful. */
   ado->timehistory = NULL;

/*****  Options ***********/
  /* These need to be options put into the DDAML DTD */
  /* These options moved in here from compilecontrol.c */
   //ado->seismicflag = 0; //FALSE;
   ado->solvetype = lu;
  /* Not really implemented yet...  Next step is to set the
   * initialize the lagrange variables before the open-close
   * iteration starts for any particular time step.
   */
   //AData->contactmethod = auglagrange; 
   ado->contactmethod = penalty; 
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

   ado->free = adata_delete; //freeAnalysisData;


  /* FIXME: The xml input file has this as a user controlled 
   * parameter.  8 is default value from GHS code.
   */
   if (ado->OCLimit == 0)
      ado->OCLimit = 6+2;

   return ado;
}  


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


