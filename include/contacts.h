/*
 * contacts.h
 *
 * First pass at getting contacts separated from 
 * the DDA code.
 */

#ifndef __CONTACTS_H__
#define __CONTACTS_H__



#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


/* First pass is an array wrapper.  Later,
 * each contact will have its own struct.
 */
typedef struct _contacts Contacts;

/* Struct for each contact. Not currently used. */
typedef struct _C C;

Contacts * contacts_new            (int nblocks);

void       contacts_delete         (Contacts *);


double     compute_length          (const double x1, 
                                    const double y1, 
                                    const double x2, 
                                    const double y2);

double     compute_length_squared  (const double x1, 
                                    const double y1, 
                                    const double x2, 
                                    const double y2);


void       contacts_compute_bboxen (double ** bbox_c0,
                                    double ** vertices, 
                                    int numblocks, 
                                    int ** vindex);


int        contacts_bbox_overlap   (double ** bbox_c0, 
                                    int ii, 
                                    int jj, 
                                    double d0);




void updateLockState(C *);

int    ** get_locks(Contacts *);
int    ** get_contacts(Contacts *);
int    ** get_contact_index(Contacts *);
double ** get_contact_lengths(Contacts *);
int    ** get_previous_contacts(Contacts *);


#ifdef __cplusplus
}
#endif


#endif  /* __CONTACTS_H__ */