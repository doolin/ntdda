
/** This procedure reads in the data from the 
 * .ana files in original ghs format
 *
 * $Author: doolin $
 * $Date: 2002/08/03 14:42:29 $
 * $Source: /cvsroot/dda/ntdda/src/analysisreader.c,v $
 * $Revision: 1.7 $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//#include "analysisdata.h"
#include "analysis.h"
#include "ddamemory.h"



void
analysisReader(Analysisdata * adn, char * af, int nfp, int pc, int nlp) {

   FILE * analysisFile;
   int i, j, n7, n8;
   int jointmat;  /* Loop counter to initialize joint materials matrix. */
   double ** materialProps;
   double ** phiCohesion;
   double ** timeDeps;
   int i1;

   int nFPoints = nfp;  
   int pointCount = pc;  
   int nLPoints = nlp;  



  /**************************************************/
  /* k5: index of u0               >=2 time points  */
  /* k5: 0 start of point i   1 end of point i      */
  /* k5[nFPoints+nLPoints+1][2]                     */
   int **k5;
  /*------------------------------------------------*/
  /* c : movement u v of fixed measured load points */
 	/* c[pointCount+1][3]                            */
   double **c;

  /**************************************************/
  /* k5: index of u0               >=2 time points  */
  /* k5: 0 start of point i   1 end of point i      */
  /* k5[nFPoints+nLPoints+1][2]                     */
   adn->k5size1 = nFPoints+nLPoints+1;
   adn->k5size2 = 2;
   k5 = IntMat2DGetMem(adn->k5size1, adn->k5size2);
  /* c : movement u v of fixed measured load points */
  /* c[pointCount+1][3]                             */
  /* It appears that c[][] is used as a flag in 
   * analysisreader(2) to indicate when to read in values
   * for time dependent properties, then later used to 
   * record movement of measured, fixed, and load
   * points.
   */
   n7=pointCount+1;
   n8=3;
   adn->csize1 = n7;
   c = DoubMat2DGetMem(n7, n8);

   analysisFile = fopen(af, "r");

   fscanf(analysisFile,"%d",&adn->analysistype);
   fscanf(analysisFile,"%d", &adn->nTimeSteps);
   fscanf(analysisFile,"%d", &adn->nBlockMats);
   fscanf(analysisFile,"%d", &adn->nJointMats);
   fscanf(analysisFile,"%lf",&adn->maxdisplacement);
   fscanf(analysisFile,"%lf",&adn->maxtimestep); /* reads value but ignores it! */


  /* Save the following for reimplementation of time-
   * dependent parameters.
   */
  /* dynamic diplacement and loading input          */
  /* k5[i][0] >= 2                                  */
   for (i=1; i<= nFPoints+nLPoints; i++) {
     /* What this means is that the c matrix is always set 
      * to unity in the next loop.
      */
	     k5[i][0] = 0; /* no time-dependent loads or displacements */
   }  /*  i  */

      
  /* (GHS: k5[i][0]=0 fixed points   c[i][0]=1 flag) */
   for (i=1; i<= nFPoints+nLPoints; i++) {

      if (k5[i][0] == 0)
      {
         c[i][0]=1;
         k5[i][0]=2;
      } 
      else
      {
         c[i][0] = 0;
      }
   } 

  /* (GHS: k5[i][0] start     k5[i][1]  end) */
   i1=0;
   for (i=1; i<= nFPoints+nLPoints; i++)
   {
      i1 += k5[i][0];
      k5[i][1] = i1;
      k5[i][0]  = k5[i][1] - k5[i][0] + 1;
   }  /*  i  */
  /* Set nt to the accumulated value...
   */
   adn->nt=k5[nFPoints+nLPoints][1]; 


  /**************************************************/
  /* u0: t x y of time dependent displacements      */
  /* u0: t x y of time dependent loading            */
  /* u0[nt+1][3]                                    */
   //n7=adn->nt+1;
   //n8=3;
   //timeDeps = DoubMat2DGetMem(n7, n8);
   adn->timedepsize1=adn->nt+1;
   adn->timedepsize2=3;
   timeDeps = DoubMat2DGetMem(adn->timedepsize1, adn->timedepsize1);

  /**************************************************/
  /* ma we e0 u0 s11 s22 s12 t11 t22 t12 vx vy vr   */
  /* a0[nb+1][13]                                   */
  /*
   n7=adn->nBlockMats+1;
   n8=13;
   */
   n7 = adn->materialpropsize1 = adn->nBlockMats + 1;
   n8 = adn->materialpropsize2 = 13;
   materialProps = DoubMat2DGetMem(n7,n8);

  /*------------------------------------------------*/
  /* b0: 0 friction       1 cohesion      2 tension */
  /* b0[nJointMats+1][3]                             */
   n7=adn->nJointMats+1;
   n8=3;
   adn->phicohesionsize1 = n7;
   adn->phicohesionsize2 = n8;
   phiCohesion = DoubMat2DGetMem(n7, n8);
  
  /**************************************************/
  /* u0: t x y of time dependent displacements      */
  /* u0: t x y of time dependent loading            */
  /* u0[nt+1][3]                                    */

   i1  = 0;

   for (i=1; i<=nFPoints+nLPoints; i++)
   {
      //if (c[i][0] < 0.5) 
        // goto a201;
      /* Since displacement dependent properties are not
       * implemented in this version, the following if statement
       * is always evaluated, the else is never reached.
       */
      if(c[i][0] >= 0.5)
      {
         i1 += 1;
         timeDeps[i1][0] = 0;
         timeDeps[i1][1] = 0;
         timeDeps[i1][2] = 0;
		       i1 += 1;
		       timeDeps[i1][0] = 100000;
         timeDeps[i1][1] = 0;
         timeDeps[i1][2] = 0;
      }
 
      else 
      {
         for (j=k5[i][0]; j<=k5[i][1]; j++)
         {
            i1 += 1;
            fscanf(analysisFile,"%lf %lf %lf",
               &timeDeps[i1][0],&timeDeps[i1][1],&timeDeps[i1][2]);
         }  /*  j  */
      }
   }  /*  i  */
      
  /* ma we e0 u0 s11 s22 s12 t11 t22 t12 vx vy vr   */
  /* Note that the dimension of a is 
   * [number of block (types?)][13].
   */
   for (i=1; i<= adn->nBlockMats; i++)
   {
     /* Read in the properties for a single block (type?).
      */
      for (j=0; j < 13; j++)
      {
         fscanf(analysisFile,"%lf",&materialProps[i][j]);
      }  /*  j  */
   }  /*  i  */


  /* friction-angle   cohesion   tension-strength   */
   /* nJointMats probably is the number of JOINT TYPES, not the 
    * number of JOINTS!!!!
    */
   for (jointmat=1; jointmat<= adn->nJointMats; jointmat++)
   {
      fscanf(analysisFile,"%lf %lf %lf",
         &phiCohesion[jointmat][0],
         &phiCohesion[jointmat][1],&phiCohesion[jointmat][2]);
   }  /*  i  */
      
   fclose(analysisFile);


   adn->timeDeps = timeDeps;
   adn->materialProps = materialProps;
   adn->phiCohesion = phiCohesion;
   adn->tindex = k5;
  /* Get rid of c */
   adn->c = c;

  /* Kludge kludge kludge:  The original file format has no 
   * notion of constant time step.
   */
   adn->autotimestepflag = 1;
   adn->autopenaltyflag = 1;
  /* The ddaml file allows the penalty number to be 
   * be adjusted.  Since this one does not, we need
   * to set the multiplier explicitly.
   */
   adn->pfactor = 50.0;

   /* New file format does this automatically. */
   adn->FPointSpring = adn->contactpenalty;
   constants_init(adn->constants,adn->maxdisplacement);
   adn->initialconstants = constants_clone(adn->constants);

}  /*  Close analysisReader1()  */



/* fgets pound comments:  Handle comments embedded into 
 * input files.
 */
/* FIXME: This needs to be abstracted out. */
int
fgetspc(char *paramstring, int bufsize, FILE * infile, int linenumber) {

  /* Grab a line, check the first character for a 
   * comment marker, spin until no comment marker in 
   * first position.
   */
   do { 
     fgets(paramstring, bufsize, infile);linenumber++;
   } while (paramstring[0] == '#');

   return linenumber;

}