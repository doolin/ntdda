
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
   const int numprops = 10;
   Material * m = (Material*)malloc(sizeof(Material));

   m->numblocks = numblocks;
   m->numprops  = numprops;

   m->props = (double**)malloc((numblocks+1)*sizeof(double*));
  
   for (i=1; i<=numblocks; i++) {
      m->props[i] = (double*)malloc((numprops+1)*(sizeof(double)));
   }

   return m;
}


Material * 
material_clone (Material * m) {

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



#ifdef __cplusplus
}
#endif