
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
static int changeGridSpacing(HWND,LPARAM,WPARAM);
static int handleWMCommand(HWND, LPARAM, WPARAM);
static void handleLButtonDown(HWND hDlg, LPARAM lParam);
static void handleLButtonUp(HWND hDlg, LPARAM lParam);
static void handleMouseMove(HWND, LPARAM, WPARAM);
static void handlePaint(HWND);
static void freePointList();
static void freeJointList();
static void freeBoltList();

static void drawSinglePoint(HDC, DPoint *);
static int saveData(HWND hDlg);

/* Mouse handling functions */
static void addJoint(HWND);
static void addPoint(HWND);
static void addBolt(HWND);


/* These will probably mutate into general purpose functions.
 * They should probably be moved into a different header file,
 * and have a return value.
 */
void transferJointlistToGeomStruct(Geometrydata *, JOINTLIST *);   
void transferPointlistToGeomStruct(Geometrydata *, POINTLIST *); 
void transferBoltlistToGeomStruct(Geometrydata *, BOLTLIST *);   
