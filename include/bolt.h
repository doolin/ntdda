
#ifndef __BOLT_H__
#define __BOLT_H__

#include "ddadlist.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


typedef struct _bolt Bolt;

//typedef DList Boltlist; 
typedef struct _boltlist Boltlist; 

typedef struct _boltmat BoltMat;
/** @todo  Move this into the bolt.[ch] files.
 */
//#if 0
struct _boltmat{
   double e00;
   double t0;
   double f0;
};
//#endif


Bolt * bolt_new            ();
void   bolt_delete         (Bolt *);


void   bolt_set_endpoints  (Bolt * b, 
                            double x1,
                            double y1,
                            double x2,
                            double y2);

void   bolt_get_endpoints  (Bolt * b, 
                            double * x1,
                            double * y1,
                            double * x2,
                            double * y2);


/** Boltlist methods, which may go into their own
 * header file in the future.
 */
Boltlist * boltlist_new        (void);

void       boltlist_append     (Boltlist *, 
                                Bolt *);

int        boltlist_length     (Boltlist *);

void       boltlist_get_array  (Boltlist *, 
                                double ** boltarray);



#ifdef __cplusplus
}
#endif

#endif /* __BOLT_H__ */
