/*
 * contacts.h
 *
 * First pass at getting contacts separated from 
 * the DDA code.
 */

#ifndef _CONTACTS_H_
#define _CONTACTS_H_


//#include "geometrydata.h"
//#include "analysisdata.h"

/* First pass is an array wrapper.  Later,
 * each contact will have its own struct.
 */
typedef struct _contacts Contacts;

/* Struct for each contact. Not currently used. */
typedef struct _C C;

Contacts * getNewContacts(int nblocks);
void * freeContacts(Contacts *);

int    ** get_locks(Contacts *);
int    ** get_contacts(Contacts *);
int    ** get_contact_index(Contacts *);
double ** get_contact_lengths(Contacts *);
int    ** get_previous_contacts(Contacts *);


/* new stuff */
void updateLockState(C *);

#endif