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


void    toolbar_init       (HWND hwMain, 
                            HWND * htoolbar);

void    toolbar_set_state  (HWND htoolbar, 
                            unsigned int state);


#ifdef __cplusplus
}
#endif

#endif  /* __TOOLBAR_H__ */