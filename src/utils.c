/*
 * utils.c
 *
 * General utility routines for the DDA analysis
 * procedure.  Will include area, matrix multiplication
 * matrix inverse, etc.
 *
 * $Author: doolin $
 * $Date: 2002/10/11 15:44:45 $
 * $Source: /cvsroot/dda/ntdda/src/utils.c,v $
 * $Revision: 1.16 $
 */


#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "analysis.h"
#include "contacts.h"
#include "ddamemory.h"
#include "utils.h"
#include "datalog.h"


/** Maybe only used once in this file.  Try to get rid of it.
 */
extern Datalog * DLog;



/* Readjust delta_t and springstiffness sizes if
 * necessary.  I am not sure if this code works 
 * correctly, that is, the same way it worked when 
 * it was in the main analysis loop.
 * FIXME: Write some test cases to drive the 
 * checkParameters() function.
 */
int 
checkParameters(Geometrydata * gd, Analysisdata * ad, Contacts * ctacts, FILE * fp) {

   int ** locks = get_locks(ctacts);
   double ** contactlength = get_contact_lengths(ctacts);
   double g3 = constants_get_norm_spring_pen(ad->constants);


  /* after 6+2 interations reduce time interval by .3 */
  	if ((ad->m9) == ad->OCLimit /* (6+2) */ ) 
   {
      if (ad->autotimestepflag)
      {
         //fprintf(fp.logfile, "OC limit reached, cutting time step size (checkparams) from %f ",
         //                     ad->delta_t);
         ad->delta_t *= .3;
         fprintf(fp," to %f\n",ad->delta_t);
         fflush(fp);

      
         if(ad->delta_t < ad->maxtimestep/100.0)
            ad->delta_t = ad->maxtimestep/100.0;
/*
         if(ad->delta_t < ad->maxtimestep/1000.0)
            ad->delta_t = ad->maxtimestep/1000.0;
*/
         DLog->timestepcut[ad->cts]++;
      }  
      else
      {  
         //sprintf(iface->message,"Max OC count (%d) reached in time step %d",ad->OCLimit, ad->currTimeStep);
         //iface->displaymessage(iface->message);
      }
            
      return TRUE;
         //goto a001;
         //goto newtimestep;
   }

  /* It appears to me that this next code just makes sure 
   * that the time step is not greater than 3.  And if it is 
   * take sqare root of time step to get new time step, 
   * then continue.  FIXME: Find out what the [][1] entry is
   * before messing with this block.
   */
  /* (GHS: displacement ratio > 2.5 reduce time interval).
   * This parameter is set in df24(). 
   */
  /* FIXME: What inna world is this for? */
   if (ad->globalTime[ad->cts][1]>3.0) {

      fprintf(fp,"Max displacement passed on timestep %d, cutting time step size from %f ", 
              ad->cts, ad->delta_t);
      (ad->delta_t)/=sqrt(ad->globalTime[(ad->cts)][1]);
      fprintf(fp,"to %f\n",ad->delta_t);
      fflush(fp);

      DLog->timestepcut[ad->cts]++;

     /* Compute a minimum time step. */
      if(ad->delta_t < ad->maxtimestep/100.0) {

         ad->delta_t = ad->maxtimestep/100.0;
      }


      return TRUE;
      //goto newtimestep;  /* goto a001; */
   }  /* end cut time step */


  /* w6 is first set here, and is used in step() for 
   * time steps past the first.
   */
   ad->w6 = computeSpringStiffness(gd, ad, locks,contactlength);
  /* norm_spring_pen (g3) is normal penetration distance limit 
   * Basically, too much penetration so increase the
   * contact penetration spring stiffness and redo(?)
   * the time step.
   * FIXME: Change the 2.5 to a user specified value.
   */
   //if (ad->w6/(ad->constants->norm_spring_pen)  > 2.5) 
   if (ad->w6/g3  > 2.5) 
   {
     /* g0 is spring stiffness */
      //ad->g0 *= 10.0; 
     /* FIXME: Turn this off and run "concave".  The block
      * has a nice backwards travel for a few time steps.
      * Explain why this is.
      */
      ad->contactpenalty *= 10.0;
         
     /* Save the value of the spring stiffness for later
      * examination.  But w6 is just a comparison value?
      * should save AData->g0?
      */
      //ad->springstiffness[ad->currTimeStep] = ad->w6;
      //ad->springstiffness[ad->currTimeStep] = ad->g0;
      ad->springstiffness[ad->cts] = ad->contactpenalty;
     /* When this goto is eliminated, check to 
      * make sure the time step value is not incremented.
      */
      return TRUE;
      //goto newtimestep;
      //goto a001;  /* Last goto.  */
   }         

   //ad->springstiffness[ad->currTimeStep] = ad->g0;
   ad->springstiffness[ad->cts] = ad->contactpenalty;

   return FALSE;  /* Passed, on to new time step */

}  /* close checkParameters() */





/**************************************************/
/* sign: return sign                         0004 */
/**************************************************/
double sign(double f2)
{
   if (f2 > 0) 
      return ( 1.0);

   if (f2 < 0) 
      return (-1.0);

   if (f2 == 0)
      return ( 0.0);

   return (0.0);

}  /* close sign() */


/**************************************************/
/* spri: compute stiffness of contact spring 0007 */
/**************************************************/
double  
computeSpringStiffness(Geometrydata *gd, Analysisdata *ad, int **m0, double **o)
{
  /* loop over every contact */
   int nContacts = gd->nContacts;  /* Number of contacts.  */
  /* loop counter variable */   
   int contact; 
  /* maximum contact penetration */
   double w5;
  /* minimum contact penetration */
   double w6; 
  /* g2 is maximum displacement per time step. */
   //double g2 = ad->g2;
   double g2 = ad->maxdisplacement;
   double g3 = constants_get_norm_spring_pen(ad->constants);
  /* w0 is problem domain scaling constant */
   double w0 = constants_get_w0(ad->constants);
  /* temp var storing max value */
   double b1;
  /* temp var storing min value */
   double b2;  /* Might be a return variable. */

  /* Return if penalty value preset for all time steps. */
   if(!ad->autopenaltyflag)
      return 0;
 
  /* compute maximum  and minimum spring penetration */
  /* w6 is a return variable.
   */
   w6 = 0;
   w5 = 0;
   for (contact=1; contact<= nContacts; contact++)
   {
      if (m0[contact][2] == 0) 
        /* Then no contact */
         continue;   //goto sp01;
      if (o[contact][0]  > w5) 
         w5 = o[contact][0];  /* Find max */
      if (o[contact][0]  < w6) 
         w6 = o[contact][0];  /* Find min */
//sp01:;
   }  /*  i  */
   
  /* Rescale penetration values with respect to
   * actual size of problem domain.
   */
   w5 =  w5/w0;
   w6 = -w6/w0;

  /* Compute comparison values.  b1 relates the amount of
   * penetration to the maximum allowed displacement for 
   * a given time step.
   */ 
   //b1 = (ad->g0)*w5/(2.0*g2) + 0.0000001;
   b1 = (ad->contactpenalty)*w5/(2.0*g2) + 0.0000001;
  /* b2 relates the minimum penetration with the value
   * of the allowable spring penetration.
   */
   //b2 = (ad->g0)*w6/g3       + 0.0000001;
   b2 = (ad->contactpenalty)*w6/g3       + 0.0000001;
   
  /* (GHS: updating stiffness of support spring) */
  /* If the penetration is dominated by the allowable 
   * maximum displacement, set the allowable penetration
   * to this value.
   */
   if (b2<b1) 
      b2=b1;
  /* If the value is too low... */
   //if (b2 <= (ad->g0)/3.0)  
   if (b2 <= (ad->contactpenalty)/3.0)  
     /* then bump it up it a bit. */
      //b2 = (ad->g0)/3.0;
      b2 = (ad->contactpenalty)/3.0;

  /* Now reset the spring stiffness parameter for use 
   * elsewhere in the program.
   */
   //ad->g0 = b2;	
   //assert(b2 <= 10e9);

   ad->contactpenalty = b2;	

  /* The minimum scaled penetration is used in parameter
   * checking of open-close iteration, and for adaptively
   * updating the value of the time step.
   */
   return w6;
  
} /* Close computeSpringStuffness()  (spri)  */

		
#define DIST_PROCEDURE 0
#if DIST_PROCEDURE
/**************************************************/
/* dist: distance of a segment to a node     0014 */
/**************************************************/
/* x1  y1  x2  y2  x3  y3  d5                     */
/* Rewrite this as a MACRO and use it in df04
 * etc etc etc.
 */
void dist(void)
{
   double x4,  y4;
   double t1;
   double d5;

  /* nearest point of p0 in between p2 p3           */
  /* compute distance of p0 to nearst point in p2p3 */
   t1  =  (x1-x2)*(x3-x2) + (yi-y2)*(y3-y2);
   t1 /= ((x3-x2)*(x3-x2) + (y3-y2)*(y3-y2));
   if (t1 <= 0.0)  
      t1=0;
   if (t1 >= 1.0)  
      t1=1;
   x4  = x2 + t1*(x3-x2);
   y4  = y2 + t1*(y3-y2);
   d5  = sqrt((x4-x1)*(x4-x1) + (y4-yi)*(y4-yi));
}  /* Close dist().  */
#endif



/**************************************************/
/* dspl: block displacement matrix           0001 */
/*************************************************/
/* x10  y10  u2  v2  t[][]               i0  x  y */
/* FIXME: Change the arguments for this function so that 
 * it will be easier to test:
 * computeDisplacement(double T[][7], double tX, tY)
 * where T is the "return value", and tX,tY are 
 * \tilde X and \tilde Y respectively.  This way the 
 * moments array does not need to be continually 
 * dereferenced in this function.  In fact, computing 
 * the centroid could be done anytime the block is
 * already in memory (i.e., the array is being dereferenced
 * elsewhere for any other reason.  
 */
void 
computeDisplacement(double **moments, double T[][7], double x, double y, int i0)
{
   double x0, y0;
   double u2, v2;
   
  /* i0 is block number                         */
  /* x10,y10:center of gravity of the block         */
   x0 = moments[i0][2]/moments[i0][1];
   y0 = moments[i0][3]/moments[i0][1];


  /* u2 and v2 are temporary variables to handle linear 
   * rotation and strains.
   */
   u2      = x-x0;
   v2      = y-y0;

  /* Translations. */
   T[1][1] = 1;
   T[2][1] = 0;
   T[1][2] = 0;
   T[2][2] = 1;
  /* Linear rotations. */
   T[1][3] = -v2;
   T[2][3] = u2;
  /* Principal strains. */
   T[1][4] = u2;
   T[2][4] = 0;
   T[1][5] = 0;
   T[2][5] = v2;
  /* Shear strains. */
   T[1][6] = v2/2;
   T[2][6] = u2/2;

}   /*  Close dspl()  */



/* This function is a prime candidate for 
 * turning into a macro.
 */
void
setMatrixToZero(double **matrix, int row, int column)
{
   int i;
   //int j;

   for (i=0; i< row; i++)
   {
      /*
      for (j=0; j< column; j++)
      {
         matrix[i][j] = 0;
      } 
      */
      memset((void*)matrix[i],'0',column*sizeof(double));
   }  

}  /* close setMatrixToZero() */



/* The simplex area function depends on having the vertex coordinates
 * in a certain order so that the coordinates can be traversed in 
 * a CCW direction.  vertexInit performs this reordering at the outset,
 * for all subsequent calls to area.  Since the area function is 
 * called everytime step, initializing ahead of time saves a lot of
 * overhead.
 */
void
vertexInit(Geometrydata *gd)
{
  /* change vertices to have previous vertex of 1st vertex */
  /* This is a convenience for the simplex integration
   * in the area function.
   */
   int i, i1, i2;

   for (i=1; i<= gd->nBlocks; i++)
   {
      i1 = gd->vindex[i][1];
      i2 = gd->vindex[i][2];
      gd->vertices[i1-1][0]=gd->vertices[i2][0];
      gd->vertices[i1-1][1]=gd->vertices[i2][1];
      gd->vertices[i1-1][2]=gd->vertices[i2][2]; 
   }  /*  i  */

}  // Close vertexInit() 

/* FIXME: This function should moved to between the analysis 
 * part and the geometry part.  That way it can be cloned 
 * when the geometry data is cloned at the start of an analysis.
 */
void 
blockNumberOfVertex(Geometrydata * gd)
{
  /* initiate nn0 as block number index of vertices and u   */
  /* vertices is the vertex matrix, vindex is the indices of vertex
   * location in vertices, u is vertex angles (?).  nBlocks 
   * is the total number of blocks in the problem (?).
   */
   int i1,i2,i,j;

   gd->nn0size = gd->vertexCount+1;
   gd->nn0 = (int *)calloc(1, sizeof(int)*gd->nn0size);

   for (i=1; i<= gd->nBlocks; i++)
   {
      i1=gd->vindex[i][1];
      i2=gd->vindex[i][2];
      for (j=i1-1; j<= i2+3; j++)
      {
         gd->nn0[j]=i;
      }  /*  j  */
   }  /*  i  */

}  // Close blockNumberOfVertex()






void 
print2DMat(double mat[7][7], int m, int n, char * location, FILE * fp) {

   int i, j;

   assert ( (m!=0) && (n!=0) && (mat != NULL) );

   for ( i = 1; i < m; i++) {
      for (j=1; j< n; j++) {
         fprintf(fp,"%.4f  ", mat[i][j]);
      }
      fprintf(fp,"\n");
   }

} 


/* Assume that the file pointer is open */
void 
print2DArray(double ** mat, int m, int n, FILE * fp, char * location)
{
   int i, j;

   assert ( (m!=0) && (n!=0) && (mat != NULL) );

   fprintf(fp,"%s\n",location);

   for ( i = 0; i<m; i++)
   {
      for (j=0; j<n; j++)
         fprintf(fp,"%.4f  ", mat[i][j]);
      fprintf(fp,"\n");
   }

   fprintf(fp,"\n");

}  /* close print2DMat() */


void 
print1DIntArray(int * mat, int n, char * location, FILE * fp) {
   int i;

   assert ( (n!=0) && (mat != NULL) );

   for (i=0; i< n; i++) {
      fprintf(fp,"%d  ", mat[i]);
   }
   fprintf(fp,"\n");
}  


void 
print1DDoubleArray(double * mat, int n, char * location, FILE * fp) {
   int i;

   assert ( (n!=0) && (mat != NULL) );

   fprintf(fp,"1 D Array, from %s\n",location);

   for (i=0; i< n; i++) {
      fprintf(fp,"%f  ", mat[i]);
   }

   fprintf(fp,"\nEnd 1 D Array print\n\n");
}  






/* FIXME: This function may be a duplicate of the clone matrix
 * function.  If so, get rid of this in favor of clone
 * matrix.
 */
int 
copy2DDoubMat(double ** mat1, double ** mat2, int n, int m)
{
   int i, j;

   for (i=0; i<n; i++)
      for (j=0; j<m; j++)
         mat1[i][j] = mat2[i][j];
   
   return 1;

}  /*  close copy2DDoubMat() */


int 
copy2DIntMat(int ** mat1, int ** mat2, int n, int m)
{
   int i, j;

   assert( mat1 != NULL  || mat2 != NULL);

   for (i=0; i<n; i++)
      for (j=0; j<m; j++)
         mat1[i][j] = mat2[i][j];

   return 1;

}  /*  close copy2DDoubMat() */



void
initBlockProperties(Geometrydata * gd, Analysisdata * ad, double ** e0)
{
   int i, i1;
   int nBlocks = gd->nBlocks;
   int ** vindex = gd->vindex;
   double ** materialProps = ad->materialProps;
   double ** moments = gd->moments;
   int nb = ad->nBlockMats;

   double gravaccel = adata_get_grav_accel(ad);

  /* materialProps: ma we e0 u0 s11 s22 s12 t1 t2 t12 vx vy vr */
  /* (GHS: e0: ma we e0 u0 c11 c22 c12 t-weight)  */
  /* (GHS: v0: velocity of u v r ex ey gxy parameters) */
   for (i=1; i<= nBlocks; i++)
   {
     /* WARNING: The material types for the blocks must be specified
      * in order. Example: 1,2,3,4.  Any out-of-order material type,
      * such as 1,2,3,5, will result in material type 5 being demoted
      * to material type 3.  The reason for this is that the material
      * types are handled in sequential arrays.  It may be possible 
      * to over-allocate e0 to handle non-successive material types,
      * but that is a project for the future.  A better implementation
      * would hash the material type under a non-integer key.
      */
     /* FIXME: Determine if ad->materialProps is ever updated.  If not,
      * it only exists to make the transfer from the input file to
      * e0, and can possibly be radically rewritten.
      */
      if ( vindex[i][0] < 1  )  
         vindex[i][0]= 1;
      if ( vindex[i][0] > nb )  
         vindex[i][0]=nb;

      i1 = vindex[i][0];
      e0[i][0] = materialProps[i1][0]; /* density; unit mass */
      //e0[i][1] = materialProps[i1][1]; /* unit weight */
      /* Unit weight is now set with respect to user specified 
       * gravitational acceleration.  The tag needs to be deprecated
       * in the runtime files.
       */
      e0[i][1] = materialProps[i1][0]*gravaccel; /* unit weight */
      e0[i][2] = materialProps[i1][2]; /* Young's modulus */
      e0[i][3] = materialProps[i1][3]; /* Poisson's ratio */
     /* This is marked as a possible error in Mary's 
      * annotated code.
      */
     /* \sigma_{11} */
      e0[i][4] = materialProps[i1][4] + (moments[i][3]/moments[i][1])*materialProps[i1][7];
     /* \sigma_{22} */
      e0[i][5] = materialProps[i1][5] + (moments[i][3]/moments[i][1])*materialProps[i1][8];
     /* \sigma_{12} */ 
      e0[i][6] = materialProps[i1][6] + (moments[i][3]/moments[i][1])*materialProps[i1][9];
     /* Current mass */
      //e0[i][7] = materialProps[i1][0]*moments[i][1];
     /* z strain, needed for tracking density in plane stress. */
      e0[i][7] = 0;
     /* Damping parameter from Shi 1988 formulation. */
      e0[i][8] = materialProps[i1][13];
    
   }  /*  i  */
}  /* close initBlockProperties() */

void
initVelocity(Geometrydata * gd, Analysisdata * ad, double ** v0)
{
   int i, i1;
   double ** materialProps = ad->materialProps;
   int nBlocks = gd->nBlocks;
   int ** vindex = gd->vindex;

  /*------------------------------------------------*/
  /* v0: velocity of u v r ex ey gxy parameters     */
  /* initial velocity                               */
   for (i=1; i<= nBlocks; i++)
   {
      i1 = vindex[i][0];
      v0[i][1] = materialProps[i1][10]; /* v11 */
      v0[i][2] = materialProps[i1][11]; /* v22 */
      v0[i][3] = materialProps[i1][12]; /* vr */
     /* Most likely 4,5,6 store previous iteration values.
      * FIXME: find out what 4,5,6 are and where they are
      * initialized.  v0[][4-6] probably store strain
      * rates, as updated in df25()
      */
      v0[i][4] = 0;
      v0[i][5] = 0;
      v0[i][6] = 0;
   }  /*  i  */
}  /* close initVelocity() */


/* If the spring stiffness is not given as a parameter,
 * this function is called from initNewAnalysis().
 */
void
initContactSpring(Analysisdata * ad)
{  
   int i; 
   int nb = ad->nBlockMats;
   double a1;
   double ** materialProps = ad->materialProps;

/* This is out for the moment because the fixed point 
 * penalty is set from the contact spring penalty.
 */
//   if (ad->contactmethod == penalty)
//   {
     /* initial contact spring stiffness g0 */
      a1 = 0;
      for (i=1; i<= nb; i++)
      {
        /* That is, set the initial spring stiffness
         * equal to the highest Young's modulus...
         */
         if (a1 < materialProps[i][2]) 
            a1=materialProps[i][2];
      }  /*  i  */
     /* then reset it to be 50 times stiffer than the 
      * highest Young's modulus.
      */
      //ad->g0 = 50.0*a1;

     /* WARNING!!!  This may be reduced to run very stiff
      * problems to try and reduce elastic rebound.
      */
     /* pfactor gets read in from a file */
      ad->contactpenalty = ad->pfactor*a1;

      //ad->springstiffness[0] = ad->g0;
      ad->springstiffness[0] = ad->contactpenalty;

}  /* close initContactSpring() */




/*
 * WARNING!!!  This function is invoked through an analysis struct
 * function pointer that is set when the analysis data is initialized.
 * It should only be called in tandem with deleteBlock.
 */
void
deleteBlockMaterial(Analysisdata * ad, int blocknumber)
{
   int i,j,k;
   int mpsize1 = ad->materialpropsize1;  //  nBlocks
   int mpsize2 = ad->materialpropsize2;  //  indexing from 0
   double ** e0 = ad->materialProps;

   j = 0;
   for (i=1;i<=mpsize1;i++)
   {
       if (i == blocknumber)
       {
          j++;
          ad->materialpropsize1--;
          continue;
       }

       for (k=0;k<=mpsize2;k++)
       {
          e0[i-j][k] = e0[i][k];
       }
    }

   /* Set the memory in what was the last element to garbage to make
    * sure it isn't accidently used.
    */ 
    memset((void*)e0[mpsize1],0xDA,mpsize2*sizeof(double));

}  /* close deleteMaterial() */






void allocateAnalysisArrays(Geometrydata * GData, 
                   int ** kk, int ** k1,
                   double *** c0, double *** e0,
                   double *** U, int *** n) 
{

  /* k : index of a    2*block contact + 20*nBlocks      */
  /* k : reserved positions for non-zero storage    */
 	/* kk[nBlocks*40+1]                                     */
   __kksize1 = FORTY*GData->nBlocks+1;
  /* Changed to calloc so that unused entries would be zero. 
   * This makes it easier for printing out the entries for  
   * examination.
   */
  *kk = (int *)calloc(1,sizeof(int)*__kksize1);

  /*------------------------------------------------*/
  /* k1: i old block number k1[i] new block number  */
  /* k1[nBlocks+1]                                  */
   __k1size1=GData->nBlocks+1;
  *k1 = (int *)calloc(1,sizeof(int)*__k1size1);

  /*------------------------------------------------*/
  /* c0: xl xu yl yu block box in 3000              */
  /* c0: u v r ex ey gxy of sliding friction force  */
  /* c0[nBlocks+1][7]                               */
   __c0size1=GData->nBlocks+1;
   __c0size2=7;
  *c0 = DoubMat2DGetMem(__c0size1, __c0size2);

  /**************************************************/
  /* e0: block material constants                   */
  /* e0: ma we e0 u0 c11 c22 c12 t-weight           */
  /* e0[nBlocks+1][7]  e_z                          */
  /* e0[][8] = damping.                             */
  /* e0[][9] = initial area                         */
   __matpropsize1=GData->nBlocks+1;
   __matpropsize2=10;//9;//8;
  *e0 = DoubMat2DGetMem(__matpropsize1, __matpropsize2);

  /*  u v  or x+u y+v  of vertices in vertices          */
  /* u[vertexCount+1][3]                                */
   __Usize1=GData->nBlocks+1; //GData->vertexCount+1;
   __Usize2=13;
  *U = DoubMat2DGetMem(__Usize1, __Usize2);

  /**************************************************/
  /* n : 1 begin number of k a b for i-th row       */
  /* n : 2 number of k a b       for i-th row       */
  /* n : 3 limit of k element number of i row       */
  /* n[nBlocks+1][4]                                */
   __nsize1=GData->nBlocks+1;
   __nsize2=4;
  *n = IntMat2DGetMem(__c0size1, __c0size2);

}  


void deallocateAnalysisArrays(int *kk, int *k1, 
                     double **c0, double **e0, 
                     double **U, int ** n) 
{
   extern void freeCGTemp();


   freeSolverTempArrays();
   freeStorageTempArrays();
   freeContactTempArrays();
   freeIntTempArrays();
   //freeCGTemp();
   freeIntegrationArrays();


   if (n)
      free2DMat((void **)n, __nsize1);
	  if (c0)
      free2DMat((void **)c0, __c0size1);
   if (e0)
      free2DMat((void **)e0, __matpropsize1);
   //if (v0)
   //   free2DMat((void **)v0, __v0size1);
		 //if (moments)
      //free2DMat((void **)moments, __momentsize1);
   if (U)
      free2DMat((void **)U, __Usize1);

  /* 1D arrays  */
   if (kk)
   {
      free(kk);  
      kk = NULL;
   }
		 
   //if (k3)
   //{
   //   free(k3);  
   //   k3 = NULL;
   //}

  	if(k1)
   {
      free(k1);  
      k1 = NULL;
   }

}  




void
computeMass(double * mass, double ** moments, double ** e0, int numblocks) {

   int i;
   double currentarea,thickness,density;

   for (i=1; i<=numblocks; i++) {

      currentarea = moments[i][1];
      thickness = 1/(1+e0[i][7]);
      density = e0[i][0];
      mass[i] = currentarea*thickness*density;
   }
} 


