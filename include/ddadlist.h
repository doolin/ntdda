
#ifndef _DDADLIST
#define _DDADLIST

/* Doubly linked lists:  These are routines that you can use to create, use,
   and manipulate doubly linked lists.  Each list is made of Dlist structures.
   The first one is a "sentinel".  In other words, it holds nothing but points
   to the first and last elements in the list proper.  Sentinels are used
   to make programming easier - in this case, it makes deleting elements, and
   dealing with empty lists much easier.


   Dlist l, tmp;

   l = make_dl();

   When you call make_dl(), an empty list is returned.  This list has
   one Dlist element:  the sentinel.  The "flink" and "blink" pointers
   point to itself, and the "val" pointer is null.

   If you want to insert the two strings "1" and "2" into this list
   (suppose the list is in the variable l), you do the following

   dl_insert_b(l, "1");
   dl_insert_b(l, "2");

   Now, the list looks as follows:

   l --> |-------|   |-------|   |-------|
-------->| flink |-->| flink |-->| flink |-------
---------| blink |<--| blink |<--| blink |<------
         | val=0 |   |  val  |   |  val  |
         |-------|   |---|---|   |---|---|
                         |           |
                          \           \
                           --> "1"     --> "2"
    
   Suppose we want to print out the list.  Then we could do the
   following:

   for (tmp = l->flink; tmp != l; tmp = tmp->flink) {
     printf("%s\n", tmp->val);
   }

   Note how having a sentinel element makes life easier.  If the 
   list is empty, the loop works perfectly.  In this header file,
   there is a macro defined:  dl_traverse.  Look at it and make 
   sure you know how it works.  The above loop can be rewritten using
   dl_traverse:

   dl_traverse(tmp, l) printf("%s\n", tmp->val);

   The following will also work:

   dl_traverse(tmp, l) {
     printf("%s\n", tmp->val);
   }

*/




typedef struct dlist {
  struct dlist *flink;
  struct dlist *blink;
  void *val; 
} DList;

/* Nil, first, next, and prev are macro expansions for list traversal 
 * primitives. */

#ifndef nil
#define nil(l) (l)
#endif

#ifndef first
#define first(l) (l->flink)
#endif

#ifndef last
#define last(l) (l->blink)
#endif

#ifndef next
#define next(n) (n->flink)
#endif

#ifndef prev
#define prev(n) (n->blink)
#endif

/* These are the routines for manipulating lists */

extern DList * make_dl();   /* This makes an empty list */

extern void dl_insert_b(DList *, void *); 
                                     /* Makes a new node, and inserts it before
                                        the given node -- if that node is the 
                                        head of the list, the new node is 
                                        inserted at the end of the list */

#define dl_insert_a(n, val) dl_insert_b(n->flink, val)
				/* Makes a new node, and inserts it after
				   the given node -- if that node is the
                                   head of the list, the new node is
                                   inserted at the beginning of the list */
DList * dlist_copy(DList *);

extern void dl_delete_node(DList *);  /* Deletes and free's a node -- do not
				    use on the head node */

extern void dl_delete_list(DList *);  /* Deletes the entire list from existance.
		   		       Use only on the head node */

extern void *dl_val(DList *);   /* Returns node->val (used to shut lint up).
			 	 Do not use on the head node */

/* This is a macro that traverses a list.  Look this one over, and be
 * sure you're convinced of how it works 
 */
#define M_dl_traverse(ptr, list) \
  for (ptr = first(list); ptr != nil(list); ptr = next(ptr))

/* This is a macro to test whether a list is empty. */
#define dl_empty(list) (list->flink == list)

/* This function counts nodes in a list. (dmd)  */
extern int dlist_length(DList *);

#endif /* _DDADLIST */