
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


typedef enum TOOLTYPE {joint = 0, fixedpoint, measpoint, loadpoint, 
                       holepoint, bolt};


typedef DList JOINTLIST;
typedef DList POINTLIST; 
typedef DList BOLTLIST; 

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


/* These will probably mutate into general purpose functions.
 * They should probably be moved into a different header file,
 * and have a return value.
 */
void transferJointlistToGeomStruct(Geometrydata *, JOINTLIST *);   
void transferPointlistToGeomStruct(Geometrydata *, POINTLIST *); 
void transferBoltlistToGeomStruct(Geometrydata *, BOLTLIST *);   
