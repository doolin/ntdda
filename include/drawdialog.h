
#define STRICT
#include<windows.h>
#include<stdio.h>
#include<windowsx.h>
#include"dda.h"
#include"resource.h"
#include "ddadlist.h"
#include "ddamemory.h"
#include "ddatypes.h"
#include "geometrydata.h"

#include "bolt.h"


typedef enum TOOLTYPE {joint = 0, fixedpoint, measpoint, loadpoint, 
                       holepoint, bolt};


typedef DList JOINTLIST;
typedef DList POINTLIST; 
//typedef DList BOLTLIST; 

void handleInit(HWND, WPARAM wParam, LPARAM lParam);
void handleRockBolts(HWND);
void drawGridLines(HDC, int color);
void drawJoints(HDC);
void drawPoints(HDC);
void handleGridOn(HDC);
void handleGridOff(HDC);
void handleRemoveJoints(HWND, HDC hdc);
void handleRemovePoints(HWND, HDC hdc);
double Scale (int, int);
void handleCancel(HWND);
int handleSave(HWND);


