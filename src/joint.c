
#include <stdlib.h>
#include <string.h>

#include "joint.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



/** @todo Add code for supporting Joints as
 * incomplete types.
 */


/**
 * @todo Figure out a way to "handle" exceptions 
 * in C without resorting to ugly macros, then
 * use one here to throw when a joint value is
 * out of range.
 */
int 
joint_check_range(double value, double min, double max) {

   if ( (value >= min) &&
        (value <= max)) {
        return 1;
   } else {
        return 0;
   }
}


Jointmat * 
jointmat_new (void) {

   Jointmat * jm = (Jointmat*)malloc(sizeof(Jointmat));
   memset(jm,0x0,sizeof(Jointmat));
   return jm;
}


/** @todo Use a linked list here instead of an array. */
Jointmat *
jointmat_array_new (int numjoints) {

   Jointmat * jm;
   jm = (Jointmat *)calloc(numjoints,sizeof(Jointmat));
   return jm;
}


void       
jointmat_delete (Jointmat * jm) {

   free(jm);
}


double     
jointmat_get_friction  (Jointmat * jm) {

   return jm->friction;
}


void       
jointmat_set_friction  (Jointmat * jm, double friction) {

   jm->friction = friction;
}


double     
jointmat_get_cohesion  (Jointmat * jm) {

   return jm->cohesion;
}


void       
jointmat_set_cohesion  (Jointmat * jm, double cohesion) {

   jm->cohesion = cohesion;
}


double     
jointmat_get_tension   (Jointmat * jm) {

   return jm->tension;
}


void       
jointmat_set_tension   (Jointmat * jm, double tension) {

   jm->tension = tension;
}


int        
jointmat_get_type      (Jointmat * jm) {

   return jm->type;
}

void       
jointmat_set_type      (Jointmat * jm, int type) {

   jm->type = type;
}

#ifdef __cplusplus
}
#endif