


#ifndef _REPLAY_H_
#define _REPLAY_H_

#include <windef.h>
#include "dda.h"
#include "graphics.h"



int handleResultsViewReplay(HWND hwMain, LPARAM lParam, GRAPHICS *);

void initReplayFile(Geometrydata * gd, Analysisdata * ad);

void writeReplayFile(Geometrydata * gd, Analysisdata * ad);

void readReplayFile(HWND hwMain, GRAPHICS * g, char *replayfilename);


#endif

