#include "contacts.h"
#include "ddafile.h"



/* The point of this is to run it through debauch on
 * the unix side and see if there is any memory leaking.
 * More can be done later with open-close etc.
 */
int
main()
{
   Contacts * C;

   C = getNewContacts(3);

   freeContacts(C);

   return 0;

}
