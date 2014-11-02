/**
 * Various macros that will be useful in DDA are built
 * and tested in this file.  Later, the macros can be
 * moved, and this file can remain as the unit test
 * code.
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

static const int PREVIOUS = 1;
static const int CURRENT = 2;
static int QQ[1][3];

#define CONTACT_STATE_UNCHANGED(qq) \
           ( (qq[0][PREVIOUS] == 0) && (qq[0][CURRENT] == 0) )

#define CONTACT_STATE_CHANGED(qq) \
           ( (qq[0][PREVIOUS] != 0)  || (qq[0][CURRENT] != 0) )



int
test_contact_state_changed() {

   printf("CONTACT_STATE_CHANGED macro unit test.  This macro\n \
          should return TRUE if either the previous or current\n\
          contacts change from open to closed or vice versa,\
          FALSE is neither contact changes.\n");

   QQ[0][PREVIOUS] = 0;
   QQ[0][CURRENT] = 0;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
          QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if (CONTACT_STATE_CHANGED(QQ)) {
      printf("FAILED, macro returned TRUE, contacts should be unchanged\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, contacts unchanged\n\n");
   }

   QQ[0][PREVIOUS] = 0;
   QQ[0][CURRENT] = 1;
   printf("PREVIOUS = 0 (unchanged), CURRENT = 1 (open to closed)\n");
   if (CONTACT_STATE_CHANGED(QQ)) {
      printf("Passed, current contact changed from open to closed\n\n");
   } else {
      printf("FAILED, current contact changed from open to close\n");
      return FALSE;
   }


   QQ[0][PREVIOUS] = 0;
   QQ[0][CURRENT] = -1;
   printf("PREVIOUS = 0 (unchanged), CURRENT = -1 (closed to open)\n");
   if (CONTACT_STATE_CHANGED(QQ)) {
      printf("Passed, current contact changed from closed to open\n\n");
   } else {
      printf("FAILED, current contact changed from closed to open\n");
      return FALSE;
   }



   QQ[0][PREVIOUS] = 1;
   QQ[0][CURRENT] = 0;
   printf("PREVIOUS = 1 (open to closed), CURRENT = 0 (unchanged)\n");
   if (CONTACT_STATE_CHANGED(QQ))
      printf("Passed, previous contact changed from open to closed\n\n");
   else 
   {
      printf("FAILED, previous contact changed from open to closed\n");
      return FALSE;
   }   


   QQ[0][PREVIOUS] = 1;
   QQ[0][CURRENT] = 1;
   printf("PREVIOUS = 1 (open to closed), CURRENT = 1 (open to closed)\n");
   if (CONTACT_STATE_CHANGED(QQ))
      printf("Passed, both contacts changed from open to closed\n\n");
   else 
   {
      printf("FAILED, both contacts changed from open to closed\n");
      return FALSE;
   }   

   QQ[0][PREVIOUS] = 1;
   QQ[0][CURRENT] = -1;
   printf("PREVIOUS = 1 (open to closed), CURRENT = -1 (closed to open)\n");
   if (CONTACT_STATE_CHANGED(QQ))
      printf("Passed, both contacts changed\n\n");
   else 
   {
      printf("FAILED, both contacts changed\n");
      return FALSE;
   }   


   QQ[0][PREVIOUS] = -1;
   QQ[0][CURRENT] = 0;
   printf("PREVIOUS = -1 (closed to open), CURRENT = 0 (unchanged)\n");
   if (CONTACT_STATE_CHANGED(QQ))
      printf("Passed, previous contact changed from closed to open\n\n");
   else 
   {
      printf("FAILED, previous contact changed from closed to open\n");
      return FALSE;
   }   


   QQ[0][PREVIOUS] = -1;
   QQ[0][CURRENT] = 1;
   printf("PREVIOUS = -1 (closed to open), CURRENT = 1 (open to closed)\n");
   if (CONTACT_STATE_CHANGED(QQ))
      printf("Passed, both contacts changed\n\n");
   else 
   {
      printf("FAILED, both contacts changed\n");
      return FALSE;
   }   


   QQ[0][PREVIOUS] = -1;
   QQ[0][CURRENT] = -1;
   printf("PREVIOUS = -1 (closed to open), CURRENT = -1 (closed to open)\n");
   if (CONTACT_STATE_CHANGED(QQ))
      printf("Passed, both contacts changed from closed to open\n\n");
   else 
   {
      printf("FAILED, both contacts changed from closed to open\n");
      return FALSE;
   }   

  
  return TRUE;

}  /* close test_contact_state_changed() */




/* TODO: Write the other 8 cases for this macro.  */
int
test_contact_state_unchanged() {

   printf("CONTACT_STATE_UNCHANGED macro unit test.  This macro\n \
          should return FALSE if either the previous or current\n\
          contacts change from open to closed or vice versa,\n\
          and TRUE if neither contact changes.\n");

   QQ[0][PREVIOUS] = 0;
   QQ[0][CURRENT] = 0;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
           QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("Passed, macro returned TRUE, neither contact changed\n\n");
   } else {
      printf("Failed, macro returned FALSE, but neither contact changed\n\n");
      return FALSE;
   }



   QQ[0][PREVIOUS] = 0;
   QQ[0][CURRENT] = 1;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
           QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("Failed, macro returned TRUE, current open to closed\n\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, current current open to closed\n\n");
   }


   QQ[0][PREVIOUS] = 0;
   QQ[0][CURRENT] = -1;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
           QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("Failed, macro returned TRUE, current closed to open\n\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, current closed to open\n\n");
   }


   QQ[0][PREVIOUS] = 1;
   QQ[0][CURRENT] = 0;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
           QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("Failed, macro returned TRUE, previous open to closed\n\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, previous current open to closed\n\n");
   }


   QQ[0][PREVIOUS] = 1;
   QQ[0][CURRENT] = 1;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
           QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("Failed, macro returned TRUE, both closed to open\n\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, both closed to open\n\n");
   }

   QQ[0][PREVIOUS] = 1;
   QQ[0][CURRENT] = -1;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
           QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("Failed, macro returned TRUE, both changed\n\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, both changed\n\n");
   }

   QQ[0][PREVIOUS] = -1;
   QQ[0][CURRENT] = 0;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
           QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("Failed, macro returned TRUE, but previous changed closed to open\n\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, previous changed from closed to open\n\n");
   }

   QQ[0][PREVIOUS] = -1;
   QQ[0][CURRENT] = 1;
   printf("PREVIOUS = %d (unchanged), CURRENT = %d (unchanged)\n",
           QQ[0][PREVIOUS], QQ[0][CURRENT]);
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("Failed, macro returned TRUE, but both changed\n\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, both contacts changed\n\n");
   }

   QQ[0][PREVIOUS] = -1;
   QQ[0][CURRENT] = -1;
   printf("PREVIOUS = -1 (closed to open), CURRENT = -1 (closed to open)\n");
   if ( CONTACT_STATE_UNCHANGED(QQ) ) {
      printf("FAILED, macro returned TRUE, but both contacts changed\n\n");
      return FALSE;
   } else {
      printf("Passed, macro returned FALSE, both contacts changed\n\n");
   }

   return TRUE;
}




int
main() {

   if (test_contact_state_changed())
      printf("Passed CONTACT_STATE_CHANGED macro\n\n");
   else
   {
      printf("Failed CONTACT_STATE_CHANGED macro\n");
      exit(-1);
   }


   if (test_contact_state_unchanged())
      printf("Passed CONTACT_STATE_UNCHANGED macro\n\n");
   else
   {
      printf("Failed CONTACT_STATE_UNCHANGED macro\n");
      exit(-1);
   }

   printf("Everything passed\n");
   return 0;
}
