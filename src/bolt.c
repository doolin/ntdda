

#include <stdlib.h>
/* memset */
#include <string.h>  
#include "bolt.h"
#include "ddadlist.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif




struct _bolt {

   int type;
   double x1,y1,x2,y2;
   double strength;
   double pretension;
};


struct _boltlist {
   DList * list;
};



Bolt * 
bolt_new() {

   Bolt * b = (Bolt*)malloc(sizeof(Bolt));
   memset((void*)b,0xda,sizeof(Bolt));
   return b;
}


Bolt * 
bolt_new_1(double x1, double y1, double x2, double y2) {

   Bolt * b = (Bolt*)malloc(sizeof(Bolt));
   memset((void*)b,0xda,sizeof(Bolt));

   b->x1 = x1;
   b->y1 = y1;
   b->x2 = x2;
   b->y2 = y2;

   return b;
}

void
bolt_delete(Bolt * b) {
   free(b);
}

void   
bolt_set_endpoints(Bolt * b,double x1,double y1, 
                   double x2, double y2) {

   b->x1 = x1;
   b->y1 = y1;
   b->x2 = x2;
   b->y2 = y2;
}


int
bolt_equals(Bolt * b1, Bolt * b2) {

  if (b1->x1 != b2->x1 ||
      b1->y1 != b2->y1 ||
      b1->x2 != b2->x2 ||
      b1->y2 != b2->y2 ) {
    return 0;
  } else {
    return 1;
  }
}

void
bolt_print(Bolt * b, PrintFunc printer, void * stream) {

  printer(stream,"Bolt printing not yet implemented.\n");
}

int
bolt_get_type(Bolt * b) {
   
   return b->type;
}

void 
bolt_set_type(Bolt * b, int type) {

   b->type = type;
}

void 
bolt_get_endpoints(Bolt * b,double * x1, double * y1,
                   double *x2,double * y2) {

   *x1 = b->x1;
   *y1 = b->y1;
   *x2 = b->x2;
   *y2 = b->y2;
}



Boltlist * 
boltlist_new() {

   Boltlist * bl = (Boltlist*)malloc(sizeof(Boltlist));
   bl->list = make_dl();

   return bl;
}


void
boltlist_delete(Boltlist * bl) {

  dl_delete(bl->list,(FreeFunc)bolt_delete);
  free(bl);
}


void
boltlist_append(Boltlist * boltlist, Bolt * bolt) {

   dl_insert_b(boltlist->list,bolt);
}

#if 0
Bolt * 
boltlist_next_bolt(Boltlist * bl) {

  //return 

}
#endif


void
boltlist_get_array(Boltlist * boltlist, double ** array) {

   int i = 0;
   DList * ptr;
   Bolt * btmp;
   double x1,y1,x2,y2;

   M_dl_traverse(ptr, boltlist->list) {

      btmp = ptr->val;

      bolt_get_endpoints(btmp,&x1,&y1,&x2,&y2);
      array[i][1] = x1;
      array[i][2] = y1; 
      array[i][3] = x2;
      array[i][4] = y2;
      //array[i+1][5] = btmp->type;
      i++;
   }
}


int
boltlist_length(Boltlist * bl) {
   return dlist_length(bl->list);
}


void 
boltlist_print(Boltlist * bl, PrintFunc printer, void * stream) {
 
   DList * ptr;
   Bolt * b;

   M_dl_traverse(ptr, bl->list) {

      b = (Bolt*)ptr->val;
      bolt_print(b,printer,stream);
   }
}


#ifdef __cplusplus
}
#endif
