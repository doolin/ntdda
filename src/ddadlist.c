/* 
 * $Source: /cvsroot/dda/ntdda/src/ddadlist.c,v $
 * $Revision: 1.5 $
 * $Date: 2002/08/03 14:42:29 $
 * $Author: doolin $
 */

#include <stdio.h> 
#include<malloc.h>

#include "ddadlist.h"

#define boolean int
#define TRUE 1
#define FALSE 0


/*---------------------------------------------------------------------*
 * PROCEDURES FOR MANIPULATING DOUBLY LINKED LISTS 
 * Each list contains a sentinel node, so that     
 * the first item in list l is l->flink.  If l is  
 * empty, then l->flink = l->blink = l.            
 *---------------------------------------------------------------------*/

DList * 
dlist_new(void) {

  DList * d;

  d = (DList *) calloc (1,sizeof(struct dlist));
  d->flink = d;
  d->blink = d;
  d->val = (void *) 0;
  return d;
}
 
void dl_insert_b(DList *node, void * val)	/* Inserts to the end of a list */

{
  DList * last_node;
  DList * new;

  new = (DList *) calloc (1,sizeof(struct dlist));
  new->val = val;

  last_node = node->blink;

  node->blink = new;
  last_node->flink = new;
  new->blink = last_node;
  new->flink = node;
}

/*  This routine puts a list in-line with an existing list. */
void
dl_insert_list_b(node, list_to_insert)
DList * node;
DList * list_to_insert;
{
  DList * last_node, * f, * l;

  if (dl_empty(list_to_insert)) {
    free(list_to_insert);
    return;
  }
  f = list_to_insert->flink;
  l = list_to_insert->blink;
  last_node = node->blink;

  node->blink = l;
  last_node->flink = f;
  f->blink = last_node;
  l->flink = node;
  free(list_to_insert);
}


/* Deletes an arbitrary iterm */
void dl_delete_node(DList * item)		
{
  item->flink->blink = item->blink;
  item->blink->flink = item->flink;
  free(item);
}

void dl_delete_list(DList * l)
{
  DList * d, * next_node;

  d = l->flink;
  while(d != l) {
    next_node = d->flink;
    free(d);
    d = next_node;
  }
  free(d);
}

void *dl_val(l)
DList * l;
{
  return l->val;
}


/* This function will crash if dlist has not
 * been initialized.  So if you find yourself here
 * as a result of a segfault, make sure that you
 * have a "dlist = make_dl()" somewhere previous
 * to this call. (Note: the "make_dl()" function 
 * may change names.  
 */
int
dlist_length(DList * dlist)
{
   DList *   tmp;
   int             i = 0;
   for (tmp = dlist->flink; tmp != dlist; tmp = tmp->flink) 
      ++i;
   
return i;
}

/*
 * Copy the contents of one dlist into another.  Calls the insert routine to
 * insert and malloc the memory for the new list.  Needs to go into header
 * file
 */

DList *
dlist_copy(DList * d1)
{
   DList * f, * d2;
   //int             i = 0;	/* Debug. */
   d2 = dlist_new();

   f = d1->blink;
   while (f != d1) {
      dl_insert_b(d2, (void *) d1->flink->val);
/*      printf("i = %d\n", ++i);	Debug.  */
      d1 = d1->flink;
   }
   return d2;
}

DList *
merge(DList * d1, DList * d2)
{
  DList *  d3;
  DList *  todo;
 
  d3 = dlist_new();
  /*
   * While 1st list doesn't point to itself or 2d list doesn't point to
   * itself, do the following:
   */
  while (d1->flink != d1 || d2->flink != d2) {
    /* If d1 is empty, todo gets d2.  */
    if (d1->flink == d1) {
      todo = d2;
      /* Likewise, if d2 is empty, todo gets d1.  */
    } else if (d2->flink == d2) {
      todo = d1;
      /*
       * Else compare the values of d1 and d2.  If d1 less than d2, todo gets
       * d1,
       */
    } else if (d1->flink->val < d2->flink->val) {
      todo = d1;
      /* ...else todo gets d2. */
    } else {
      todo = d2;
    }
    /* Don't want it in reverse order.  */
    /* dl_insert_a(d3, todo->flink->val);  */
    /*
     * Now insert the value of the first node of todo at the back of list d3.
     */
    dl_insert_b(d3, todo->flink->val);
    /* Delete the first node in list todo. */
    dl_delete_node(todo->flink);
  }                         /* End of while loop.  */
  dl_delete_list(d1);
  dl_delete_list(d2);
  return d3;
}


void
dl_delete(DList * l, FreeFunc delete) {

  DList * d, * next_node;

  d = l->flink;
  while(d != l) {
    next_node = d->flink;

    if (d->val != NULL) {
      delete(d->val);
    }
    free(d);
    d = next_node;
  }
  free(d);

}
