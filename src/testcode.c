

#include "analysis.h"
#include <string.h>

extern FILEPOINTERS fp;



void 
printConstants(CONSTANTS * c)
{

   fprintf(fp.logfile,"From printConstants():\n");
   fflush(fp.logfile);

} /* close printConstants() */



/**************************************************/
/* dc19: save block data to file                  */
/**************************************************/
/* k0 is probably the vindex matrix */
/* FIXME: Rewrite this as a function which dumps
 * the contents of a Geometrydata struct to a file.
 */
void testGeomdata(Geometrydata *gd, int **k0, double **d)
{
   int i;
   int oo;
   int n0;   /*  Number of blocks in the problem.  */
   FILE *geomdatatest;  /* Uses the rootname of the current geometry.*/
   double ** g = gd->points;
   double ** gg1 = gd->rockbolts;

   n0 = gd->nBlocks;

   /* This entire piece of code needs to be 
    * moved to somewhere else.  For now, 
    * the best place would be to move it to 
    * the top of the geometry file, or 
    * have it called from the geometry dialog
    * box.  
    */

      
  /*  For now, print them both out.
   */
   //blockOutFile = fopen("block.in", "w");
   geomdatatest = fopen("geomdata.test", "w");
		

  /* n0   number of blocks   nBolt number of bolts   */
  /* nFixed fixed points       nLoad loading points    */
  /* nMeas measured points                           */
   oo = k0[n0][2]+4;
  /* Writes to "root.blk" */
   fprintf(geomdatatest,"%d %d %d \n",   n0,gd->nBolts,  oo);
   fprintf(geomdatatest,"%d %d %d \n", gd->nFPoints,gd->nLPoints,gd->nMPoints);


  /* material number  block start  block end  index */
  /* The first (0th) array element should be the block 
   * material number.  I don't believe this is currently
   * implemented.  Also, wherever this is determined, there
   * needs to be some argument checking.
   */
   for (i=1; i<= n0; i++)
   {
      fprintf(geomdatatest,"%d %d %d \n", k0[i][0], k0[i][1], k0[i][2]);
   } /* i */
      
      
  /* x  y of each block  0  s0  sx  sy  0  0        */
   for (i=1; i<= k0[n0][2]+4; i++)
   {
      fprintf(geomdatatest,"%f %f %f\n",d[i][0],d[i][1],d[i][2]);
   } /* i */


  /* gg1: x1  y1  x2  y2  n1  n2  e00  t0  f0    bolt */
  /* n1 n2 carry block number        f0 pre-tension */
  /* This has to be indexed from 0!!!!
   */
   for (i=0; i< gd->nBolts; i++)
   {
     /* Write to the "rootname.blk" file. */
      fprintf( geomdatatest, "%f %f %f\n", gg1[i][1],gg1[i][2],gg1[i][3]);
		    fprintf( geomdatatest, "%f %d  %d\n", gg1[i][4],(int)gg1[i][5],(int)gg1[i][6]);
      fprintf( geomdatatest, "%f %f %f\n", gg1[i][7],gg1[i][8],gg1[i][9]);
   }   /* i */

      
  /* x  y  of fixed loading measured points         */
   for (i=1; i<= gd->nFPoints+gd->nLPoints+gd->nMPoints; i++)
   {
    		fprintf(geomdatatest,"%f %f %f %f \n", 
              g[i][0],g[i][1],g[i][2],g[i][3]);
   } /* i */

      fflush(geomdatatest);
      fclose(geomdatatest);
}  // Close dc19

void
printVertices(Geometrydata * gd, double ** vertices, int ** vindex, char * location)
{
   int i, j;
   int i1, i2;
  /* (GHS: change to new deformed block shape  u[][] df24) */
  /* u appears to be the displacement parameter set as a 
   * solution to the linear system.  Since the constraints
   * force an iterative procedure, u (appears) is the end result of
   * the iteration to convergence in the time steps.  For 
   * disp dep variables, this array will need to be copied into 
   * an array that holds the previous vertices, then the 
   * vertex-vertex displacement can be computed to update
   * material properties.
   */
   fprintf(fp.logfile, "Start vertex output\n");
   fprintf(fp.logfile, "Leading dim: %d\n", gd->vertexsize1);
   fprintf(fp.logfile, "From %s\n",location);
   for (i=1; i<=  gd->nBlocks; i++)
   {
      i1=vindex[i][1];
      i2=vindex[i][2];
      
      fprintf(fp.logfile,"block: %d, start %d, stop %d \n", i, i1, i2);
      for (j=i1-1; j<=i2+1; j++)
      {
         fprintf(fp.logfile,"vertex: %10d, %15.10g  %15.10g\n",
             j, vertices[j][1], vertices[j][2]);
      }  /*  j  */
      fprintf(fp.logfile,"\n");
   }  /*  i  */
   fprintf(fp.logfile, "End vertex output\n\n");

}  /* close printVertices() */


void
printForces(Geometrydata * gd, double ** F, int * k1, char * location)
{
   int i, i1;

   fprintf(fp.logfile, "\n%%  Begin block force verify output (from %s):\n", location);
   for (i=1; i<= gd->nBlocks; i++)
   {
      //i1 = k1[i];
      i1 = i;
      //fprintf(fp.logfile,"Begin block %d\n", i);
      fprintf(fp.logfile,"F(%d,:) = [ %e, %e, %e, %e, %e, %e];\n", 
               i1, F[i1][1], F[i1][2], F[i1][3], F[i1][4], F[i1][5], F[i1][6]);
      //fprintf(fp.logfile,"End block\n");
   }  /*  i  */
   fprintf(fp.logfile, "%%  End block forces\n");


}  /* close printForces() */


/* FIXME: modify to compute file offsets so that the 
 * block areas can be stored in the same file as
 * variables.  Blocks with very large area will need 
 * compile time modification of the output formatting.
 */
void 
printBlockAreas(Geometrydata * gd, Analysisdata * ad, double ** moments, 
                 char * location)
{
  /* loop counter */
   int i;
  /* declares variable or header name */
   const char headerstring[] = {"#\nblockareas = [\n"};
   const char trailerstring[] = {"#];"};
   static long headersize;
   static long trailersize;
   static long rowsize;
   static long blocksize;
   const short doublewidth = 10;
   //const short intwidth = 10;
   const short separatorwidth = 1;

  /* None of the following code is actually used for anything
   * at the moment, but it will be used in the future when 
   * the output file handling gets a little more sophisticated.
   */
   if (ad->currTimeStep == 1)
   {
      headersize = strcspn(headerstring, "\0");
      trailersize = strcspn(trailerstring, "\0");
      rowsize = (gd->nBlocks+1)*(doublewidth + separatorwidth)
              + 1/* \n */ + 1/* \0 */;
      blocksize = rowsize*ad->nTimeSteps + headersize + trailersize;
   }

   //fprintf(fp.areafile, "\nBegin block area verify output:\n");
   //fprintf(fp.areafile,"%10.5f ", ad->currentTime);
   fprintf(fp.momentfile,"%10.5f ", ad->elapsedTime);
  /* Handling a user-specified separator (i.e., comma, tab
   * space) will require using a while loop here.
   */
   for (i=1; i<=gd->nBlocks; i++)
   {
      fprintf(fp.momentfile, "%10.4f ", moments[i][1]);
   }
   fprintf(fp.momentfile, "\n");

} /* close printBlockArea() */



/* This has to be handled in the blocks.c file */
void 
printPoints(Geometrydata * gd, char * location, FILE * outfile)
{
   int i;
   double ** points = gd->points;
   int nlp = gd->nLPoints;
   int nmp = gd->nMPoints;
   int nfp = gd->nFPoints;

   fprintf(outfile,"Points array, from %s\n",location);

   for (i=0;i<=nfp+nlp+nmp;i++)
   {
       fprintf(outfile,"%f %f %f %f %f\n",points[i][0],
               points[i][1], points[i][2], points[i][3],points[i][4]);
   }
   fprintf(outfile,"\n");
}  /* close printPoints() */



void 
printBlockWeights(Geometrydata * gd, double ** moments, double ** e0,
                   char * location)
{
   int i;

   fprintf(fp.logfile, "\nBegin block weight verification:\n");
   for (i=1; i<=gd->nBlocks; i++)
   {
      fprintf(fp.logfile, "Block: %d, Weight:  %f\n", i, e0[i][1]*moments[i][1]);
   }
   fprintf(fp.logfile, "End block weights\n\n");

} /* close printBlockArea() */


void 
printGravNormForce(Analysisdata * ad, char * location)
{
   int i;
   int n2 = ad->nCurrentContacts;
   double ** cnf = ad->gravity->contactFN;


   fprintf(fp.gravfile, "\nBegin gravity normal force verification" 
           " (from %s, timestep %d, iteration %d)\n",
            location, ad->currTimeStep, ad->m9);
   for (i=0; i<=n2; i++)
   {
      fprintf(fp.gravfile, "contact: %d, %f %f %f %f\n", 
              i, cnf[i][0], cnf[i][1], cnf[i][2], cnf[i][3]);
   }
   fprintf(fp.gravfile, "End gravity normal forces\n\n");

} /* close printBlockArea() */


void 
printGravShearForce(Analysisdata * ad, char * location)
{
   int i;
   int n2 = ad->nCurrentContacts;
   double ** csf = ad->gravity->contactFS;

   fprintf(fp.gravfile, "\nBegin gravity shear force verification" 
           " (from %s, timestep %d, iteration %d)\n",
            location, ad->currTimeStep, ad->m9);

   for (i=0; i<=n2; i++)
   {
      fprintf(fp.gravfile, "contact: %d, %f %f %f %f\n", 
              i, csf[i][0], csf[i][1], csf[i][2], csf[i][3]);
   }
   fprintf(fp.gravfile, "End gravity shear forces\n\n");

} /* close printBlockArea() */


void 
printContactLengths(Geometrydata * gd, double ** o, char * location)
{
   int i;
  /* o : 0 normal penetration  1 shear  movement    */
	 /* o : 2 contact edge ratio  3 cohesion length    */
   
   fprintf(fp.logfile, "\nBegin contact length verification:\n");
   fprintf(fp.logfile, "Contact   Norm Pen.  Shear movement  Edge ratio   Cohesion length\n");
   for (i=1; i<=gd->nContacts; i++)
   {
      fprintf(fp.logfile, "%3d  %10f %10f %10f %10f %10f %10f %10f\n",
              i, o[i][0], o[i][1], o[i][2], o[i][3], o[i][4], o[i][5], o[i][6]);
   }
   fprintf(fp.logfile, "End contact length\n\n");

} /* close printBlockArea() */

void 
printAngles(Geometrydata * gd, double ** angles, char * location)
{
   int i,j,i1,i2;
   //double ** vertices = gd->vertices;
   int ** vindex = gd->vindex;
   
   fprintf(fp.logfile, "\nBegin angle verification (from %s:\n",
                         location);
   fprintf(fp.logfile, "Leading dim: %d\n", __Usize1); 
   //fprintf(fp.logfile, "Contact   Norm Pen.  Shear movement  Edge ratio   Cohesion length\n");
  /* WARNING!!!! __anglesize1 is global from analysis.h
   * This will eventually be handled using object methods.
   */

   for  (i=1;i<=gd->nBlocks;i++)
   {
      i1 = vindex[i][1];
      i2 = vindex[i][2];
      fprintf(fp.logfile, "Block %d, start %d, stop %d,\n",i,i1,i2);
      for (j=i1-1;j<=i2+1;j++)
      {
         fprintf(fp.logfile, "angle: %10d, %10.4f %10.4f\n",
                 j, angles[j][1], angles[j][2]);
      }
      fprintf(fp.logfile, "\n");
   }  /* i */

#if OLDANGLES
   for (i=0; i<__anglesize1; i++)
   {
      fprintf(fp.logfile, "%10d  %10f %10f %10f\n",
              i, angles[i][0], angles[i][1], angles[i][2]);
   }
#endif /* OLDANGLES */

   fprintf(fp.logfile, "End angle verification\n\n");

} /* close printAngles() */


void 
printContacts(int ** contacts, char * location)
{
   int i;
   
   fprintf(fp.logfile, "\nBegin contact verification:\n");
   //fprintf(fp.logfile, "Contact   \n");
  /* WARNING!!!! __contactsize1 is global from analysis.h
   * This will eventually be handled using object methods.
   */
   for (i=0; i<__contactsize1; i++)
   {
      fprintf(fp.logfile, "%d %d  %d %d %d %d %d %d\n",
              i, contacts[i][0], contacts[i][1], contacts[i][2],
                 contacts[i][3], contacts[i][4], contacts[i][5],
                 contacts[i][6]);
   }
   fprintf(fp.logfile, "End contact\n\n");

} /* close printContacts() */

void 
printContactIndex(int ** m1, char * location)
{
   int i;
   
   fprintf(fp.logfile, "\nBegin contact index verification:\n");
   //fprintf(fp.logfile, "Contact   \n");
  /* WARNING!!!! __m1size1 is global from analysis.h
   * This will eventually be handled using object methods.
   */
   for (i=0; i<__contactindexsize1; i++)
   {
      fprintf(fp.logfile, "%d  %d %d %d\n",
              i, m1[i][0], m1[i][1], m1[i][2]);
   }
   fprintf(fp.logfile, "End contact index\n\n");

} /* close printContacts() */

void printParameters(Analysisdata * ad, char * location)
{

   CONSTANTS * c = ad->constants;

   if(ad->currTimeStep == 1)
   {
         fprintf(fp.parfile,"# Analysis parameters\n");
         fprintf(fp.parfile,"# openclose opencriteria    w0     norm_spring_pen"
                            " norm_extern_dist norm_pen_dist angle_olap"
                            " shear_norm_ratio\n");
   }

   fprintf(fp.parfile,"  %8.5f %10.5f %10.5f     %8.5f   %10.5f %10.5f %10.5f %10.5f\n",
                       c->openclose/* s0 */, 
                       c->opencriteria/* f0 */,
                       c->w0/* related to the physical scale of the model. */,
                       c->norm_spring_pen /* g3  */,
                       c->norm_extern_dist /* d0 */,
                       c->norm_pen_dist  /* d9  */,
                       c->angle_olap  /*  h1  */,
                       c->shear_norm_ratio /* h2 */);

}  /* close printParameters() */

void 
printTimeDeps(double ** timedeps, int m, char * location)
{
   int i;
   
   fprintf(fp.logfile, "\nBegin time dep verification:\n");
   //fprintf(fp.logfile, "Contact   \n");
  /* WARNING!!!! __m1size1 is global from analysis.h
   * This will eventually be handled using object methods.
   */
   for (i=0; i<m; i++)
   {
      fprintf(fp.logfile, "%d   %.3f  %.3f  %.3f\n",
              i, timedeps[i][0], timedeps[i][1], timedeps[i][2]);
   }
   fprintf(fp.logfile, "End time dep\n\n");

} /* close printTimeDeps() */


void
printKForIthBlock(Analysisdata * ad, int blocki, int blockj, int * kk, int ** n,
                  char * location)
{
   double ** K = ad->K;
   //int i1,i2;
   int i3,ji,l,j;
   int startcol, stopcol;

   fprintf(fp.logfile,"Submatrix K_{%d,%d} (%s):\n",
           blocki,blockj,location);

  /* FIXME: This code does not work correctly.
   */
   if (blocki == blockj)
   {
      i3 = n[blocki][1] + n[blocki][2] - 1;
   }
   else if (blocki > blockj)  //  lower triangle
   {
     /* Find Kij */
      startcol = n[blocki][1];
      stopcol = n[blocki][1] + n[blocki][2] - 1;
      for (j=startcol; j<=stopcol;j++)
      {
         i3 = j;
         if (kk[j] == blockj)
            break;
      }
   }
   else  // upper triangle
   {
     /* Find Kji */
      startcol = n[blockj][1];
      stopcol = n[blockj][1] + n[blockj][2] - 1;
      for (j=startcol; j<=stopcol;j++)
      {
         i3 = j;
         if (kk[j] == blocki)
            break;
      }
   }


  /* (GHS: add mass matrix to a[][]) */
  /* k1 stores "permutation index" */
   //i1=k1[blocki];
   //i2=n[i1][1]+n[i1][2]-1;
  
   fprintf(fp.logfile,"K%d = [\n",blocki);

   for (j=1; j<= 6; j++)
   {
      for (l=1; l<= 6; l++)
      {
         ji=6*(j-1)+l;
         fprintf(fp.logfile,"%f ", K[i3][ji]);
      }  /*  l  */
      fprintf(fp.logfile,"\n");
   }  /*  j  */

   fprintf(fp.logfile,"]\n");

   fprintf(fp.logfile,"End print submatrix K_{%d,%d}.\n", blocki,blockj);
}  /* close printKForIthBlock() */


void 
printK(double ** K, int ksize1, char * location)
{
   int i,j;

   fprintf(fp.logfile,"\n%%Verify K (from %s)\n",location);

   for (i=1;i<ksize1;i++)
   {
      fprintf(fp.logfile,"K(%d,:) = [ ",i);
      for (j=1;j<=36;j++)
      {
         fprintf(fp.logfile," %f ",K[i][j]);
      }
      fprintf(fp.logfile," ];\n");
   }
   fprintf(fp.logfile,"%% End verify K\n");
}  /* close printK() */



void 
printKK(int * kk, int ** n, int numblocks, char * location)
{
   int i,j;
   int start, stop;

   fprintf(fp.logfile, "Begin KK partial output (From %s)\n",location);

  /* Loop over every row */
   for (i=1;i<=numblocks;i++)
   {
      start = n[i][1];
      stop = n[i][1] + n[i][2] - 1;
      fprintf(fp.logfile,"row %d: ",i);
      for (j=start;j<=stop;j++)
      {
         fprintf(fp.logfile,"%d ",kk[j]);
      }
      fprintf(fp.logfile,"\n");
   }
   fprintf(fp.logfile, "End KK partial output\n\n");


/* kk is massively overallocated.  Most of the overallocated
 * memory is used for scratch space, then, the front part of
 * it gets used for storing column numbers of blocks in contact
 * with each other.  It has to use the n[][] array for this.
 * n[i][] stores the locations in kk of the j columns in 
 * contact with row i.
 */
#if KK_FULL_OUTPUT
   fprintf(fp.logfile, "Begin KK total output (1:40*nBlocks)\n");
   for (i=1;i<__kksize1;i++)
   {
      fprintf(fp.logfile, "%d ", kk[i]);
      if (!(i%20))
         fprintf(fp.logfile, "\n");
   }
         
   fprintf(fp.logfile, "\n");
   fprintf(fp.logfile, "End KK output\n\n");
#endif

}  /* close printKK() */




#if K3
/* Moved k3 to local */
void 
printK3(int * k3, char * location)
{

int i;

   fprintf(fp.logfile, "Begin K3 output (1:40*nBlocks\n");
   fprintf(fp.logfile, "(from %s)\n",location);
   for (i=1;i<__k3size1;i++)
   {
      fprintf(fp.logfile, "%d ", k3[i]);
      if (!(i%20))
         fprintf(fp.logfile, "\n");
   }
         
   fprintf(fp.logfile, "\n");
   fprintf(fp.logfile, "End K3 output\n\n");

}  /* close printK3() */
#endif

void 
printK1(int * k1, char * location)
{

int i;

   fprintf(fp.logfile, "Begin K1 output (1:40*nBlocks\n");
   fprintf(fp.logfile, "(from %s)\n",location);
   for (i=1;i<__k1size1;i++)
   {
      fprintf(fp.logfile, "%d ", k1[i]);
      if (!(i%20))
         fprintf(fp.logfile, "\n");
   }
         
   fprintf(fp.logfile, "\n");
   fprintf(fp.logfile, "End K1 output\n\n");

}  /* close printK3() */

void 
printK5(Analysisdata * ad, char * location)
{

   int i,j;
   int m = ad->k5size1;
   int n = ad->k5size2;
   int ** tindex = ad->tindex;

   fprintf(fp.logfile, "Begin K5 output (1:40*nBlocks\n");
   fprintf(fp.logfile, "(from %s)\n",location);
   for (i=0;i<m;i++)
   {
      for (j=0;j<n;j++)
      {
         fprintf(fp.logfile, "%d ", tindex[i][j]);
      }
      fprintf(fp.logfile, "\n");
   }

   fprintf(fp.logfile, "\n");
   fprintf(fp.logfile, "End K5 output\n\n");

}  /* close printK5() */


/* n[][] is some kind of index array.
 */
void 
printN(int ** n, char * location)
{
   int i,j;

   fprintf(fp.logfile, "Begin n[][] output (nBlocks x 3)\n");
   fprintf(fp.logfile, "(from %s)\n",location);
   for (i=1;i<__nsize1;i++)
   {
      for (j=1;j<__nsize2;j++)
      {
         fprintf(fp.logfile, "%d ", n[i][j]);
      }
      fprintf(fp.logfile, "\n");
   }
         
   fprintf(fp.logfile, "\n");
   fprintf(fp.logfile, "End n[][] output\n\n");
}  /* close printN() */


/* m1[][] is some kind of contact lock index array.
 */
void 
printM1(int ** m1, char * location)
{
   int i,j;

   fprintf(fp.logfile, "Begin m1[][] output (nBlocks x 2)\n");
   fprintf(fp.logfile, "(from %s)\n",location);
   for (i=1;i<__contactindexsize1;i++)
   {
      for (j=1;j<__contactindexsize2;j++)
      {
         fprintf(fp.logfile, "%d ", m1[i][j]);
      }
      fprintf(fp.logfile, "\n");
   }
         
   fprintf(fp.logfile, "\n");
   fprintf(fp.logfile, "End m1[][] output\n\n");
}  /* close printM1() */


void
printLockStates(int ls[3][5], char * location) {

   fprintf(fp.logfile,"\n===---===\n");
   fprintf (fp.logfile,"%d %d %d %d\n",ls[1][1],ls[1][2],ls[1][3],ls[1][4]);
   fprintf (fp.logfile,"%d %d %d %d\n",ls[2][1],ls[2][2],ls[2][3],ls[2][4]);
   fprintf(fp.logfile,"\n===---===\n");

}


void 
printLocks(int ** locks, char * location)
{
   int i,j;

   fprintf(fp.logfile, "Begin locks[][] output (nBlocks x 2)\n");
   fprintf(fp.logfile, "(from %s)\n",location);
   for (i=1;i<__locksize1;i++)
   {
      for (j=0;j<__locksize2;j++)
      {
         fprintf(fp.logfile, "%d ", locks[i][j]);
      }
      fprintf(fp.logfile, "\n");
   }
         
   fprintf(fp.logfile, "\n");
   fprintf(fp.logfile, "End locks[][] output\n\n");
}  /* close printLocks() */



/* m2[][] is either previous contacts or joint materials.
 */
void 
printPreviousContacts(int ** prevcontacts, char * location)
{
   int i,j;

   fprintf(fp.logfile, "Begin prevcontact[][] output (nBlocks x 2)\n");
   fprintf(fp.logfile, "(from %s)\n",location);
  /* WARNING!!! __prevcontactsize[12] are global, defined
   * in analysis.h
   */
   for (i=1;i<__prevcontactsize1;i++)
   {
      for (j=1;j<__prevcontactsize2;j++)
      {
         fprintf(fp.logfile, "%d ", prevcontacts[i][j]);
      }
      fprintf(fp.logfile, "\n");
   }
         
   fprintf(fp.logfile, "\n");
   fprintf(fp.logfile, "End prevcontacts[][] output\n\n");
}  /* close printM1() */

//#if PRINTLOADPOINTS
void
printLoadPointStruct (Analysisdata * ad, FILE * bfp)
{
   double **lpoints;
   int i, j, n;
   FILE * ofp;

   ofp = fopen("loadpoint.log","w");

   fprintf (ofp, "\nLoad point verify\n");

   for (i = 0; i < ad->nLPoints; i++)
   {
     	n = ad->loadpoints[i].loadpointsize1;
     	fprintf (ofp, "loadpointsize1: %d\n", n);
     	lpoints = ad->loadpoints[i].vals;

     	for (j = 0; j < n; j++)
   	  {
	        fprintf (ofp, "%.4f  %.4f  %.4f\n", lpoints[j][0],
		          lpoints[j][1], lpoints[j][2]);
	     }
   }

   fprintf (ofp, "End load point verify\n\n");
   fclose(ofp);

}				/* close printLoadPointStruct() */
//#endif


void 
printVindex(Geometrydata * gd, int ** vindex, char * location)
{
   FILE * matlog;
   int i;

   matlog = fopen("mat.log","w");

   for (i=1; i<=gd->nBlocks; i++)
   {
      fprintf(matlog,"block %d material: %d\n",i,vindex[i][0]);
   }

   fclose(matlog);

}  //  close printVindex() 


/* Assume that the file pointer is open */
void 
printDinv(double ** dinv, int block, char * location)
{
   int i, j;
   int blocksize = 6;
   int index = 1;

   //assert ( (m!=0) && (n!=0) && (mat != NULL) );

   fprintf(fp.logfile,"D inverse %d from %s\n", block, location);

   for ( i = index; i<blocksize+index; i++)
   {
      for (j=index; j<blocksize+index; j++)
         fprintf(fp.logfile,"%f  ", dinv[block][blocksize*(j-index)+i]);
      fprintf(fp.logfile,"\n");
   }

   fprintf(fp.logfile,"End print D inverse\n");

}  /* close print2DMat() */