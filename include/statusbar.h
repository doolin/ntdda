
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


void statusbar_init(HWND);

void updateAnalysisStatusBar(void);
void updateGeometryStatusBar(int numblocks);


#ifdef __cplusplus
}
#endif

#endif /* __STATUSBAR_H__ */