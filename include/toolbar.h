/* 
 * prototypes for toolbar handling
 */

#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include <windef.h>

#ifdef __cpluplus
extern "C" {
#endif
#if 0
}
#endif


typedef struct _toolbar Toolbar;

void toolbar_init           (HWND hwMain);

void toolbar_show           (void);

int  toolbar_get_visibility (void);

void toolbar_set_visibility (int);

void toolbar_resize         (void);

void toolbar_set_state      (unsigned int state);


#ifdef __cplusplus
}
#endif

#endif  /* __TOOLBAR_H__ */