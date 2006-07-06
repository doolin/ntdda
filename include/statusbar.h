
#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include <windef.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/* This is ill-advised, and is only a kludge until 
 * status text size can be computed.
 */
#define STATUSBAR_TEXT_WIDTH 25


void statusbar_init             (HWND);

void statusbar_show             (void);

int  statusbar_get_visibility   (void);

void statusbar_set_visibility   (int);

void statusbar_resize           (void);

void statusbar_set_state        (unsigned int state);

void statusbar_set_text         (WPARAM wParam, 
                                 LPARAM lParam);

void statusbar_update_analysis  (int numblocks,
                                 double elapsedtime,
                                 int currtimestep, 
                                 int numtimesteps,
                                 int openclosecount);


void statusbar_update_geometry   (int numblocks);

void statusbar_update_progbar    (unsigned int timstep);

void statusbar_set_progbar_range (unsigned short value);

#ifdef __cplusplus
}
#endif

#endif /* __STATUSBAR_H__ */

