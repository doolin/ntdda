
/**
 * Encapsulate the material properties.
 */


#include <stdio.h>
#include <stdlib.h>

#include "material.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



  /**************************************************/
  /* e0: block material constants                   */
  /* e0: ma we e0 u0 c11 c22 c12 t-weight           */
  /* e0[nBlocks+1][7]  e_z                          */
  /* e0[][8] = damping.                             */
  /* e0[][9] = initial area                         */
//   __matpropsize1=GData->nBlocks+1;
//   __matpropsize2=10;//9;//8;
//  *e0 = DoubMat2DGetMem(__matpropsize1, __matpropsize2);
struct _material {

   int numblocks;
   int numprops;
   double ** props;
};



Material *
material_new (int numblocks) {

   int i;
   const int numprops = 13;
   Material * m = (Material*)malloc(sizeof(Material));

   m->numblocks = numblocks;
   m->numprops  = numprops;

   m->props = (double**)malloc((numblocks+1)*sizeof(double*));

   for (i=1; i<=numblocks; i++) {
      m->props[i] = (double*)malloc((numprops+1)*(sizeof(double)));
   }

   return m;
}

/**
 *  Probably not necessary right now.
 */
Material *
material_clone (Material * m_old) {

   return NULL;

}


void
material_delete (Material * m) {

   int i;

   for (i=1; i<=m->numblocks; i++) {
      free(m->props[i]);
   }

   free(m->props);
   free(m);
}


double **
material_get_props(Material * m) {

   return m->props;
}



void
material_validate(int ** vindex, int numblocks, int nb) {

   int i;

   for (i=1; i<=numblocks; i++) {

      if ( vindex[i][0] < 1  ) {
         vindex[i][0]= 1;
      }
      if ( vindex[i][0] > nb )  {
         vindex[i][0]=nb;
      }
   }
}




#ifdef __cplusplus
}
#endif
