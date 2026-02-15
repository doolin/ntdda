


#ifndef __REPLAY_H__
#define __REPLAY_H__

#include <windef.h>
#include "dda.h"
#include "graphics.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


int     handleResultsViewReplay (HWND hwMain,
                                 LPARAM lParam,
                                 GRAPHICS *);

void    initReplayFile          (Geometrydata * gd,
                                 Analysisdata * ad);

void    writeReplayFile         (Geometrydata * gd,
                                 Analysisdata * ad);

void    replay_analysis         (HWND hwMain,
                                 GRAPHICS * g,
                                 char *replayfilename);

void  (*replay_display_warning) (const char * message);
void  (*replay_display_error)   (const char * message);



#ifdef __cplusplus
}
#endif


#endif  /*  __REPLAY_H__ */

