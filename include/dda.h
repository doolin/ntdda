


#ifndef _DDA_H_
#define _DDA_H_

#include "geometrydata.h"
#include "analysisdata.h"
#include "ddatypes.h"


#define WINDOWS 1
/* FIXME: Move these to winmain.h */
#define WINGRAPHICS 1
#define OPENGL 0



/* These defs provide for some
 * self-documentation in the code.
 */
#define STATIC 0
#define DYNAMIC 1

#ifndef PI
#define PI 3.141592653
#endif


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif 

#define MACHEPS (3.*((4.0/3.0)-1.0)-1.0)



/* windows.h probably defines these. */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif








/* TODO: header file for functions that are common to 
 * both the Win32 code and the back end code.
 */
void error(char *);


enum inputfiletype { original = 0, extended, ddaml };

typedef enum inputfiletype IFT;


/* FIXME: move this into a better header. */
void initFilePaths(FILEPATHS *);

typedef struct gui_tag Gui;


typedef struct progbar_tag ProgressBarInfo;



int getFileType(char *);


/* Pass this around instead of using globals.
 */
typedef struct _dda_tag DDA;

struct _dda_tag {

   Geometrydata * geometry;
   Analysisdata * analysis;
   FILEPATHS * filepaths;
   int menustate;
   ddaboolean toolbarvis;
   ddaboolean statusbarvis;
   ddaboolean popupvis;
   ddaboolean tooltipvis;

};


Geometrydata * dda_get_geometrydata(DDA *);
void dda_set_geometrydata(DDA *, Geometrydata *);

Analysisdata * dda_get_analysisdata(DDA *);
void dda_set_analysisdata(DDA *, Analysisdata *);

ddaboolean dda_get_toolbarvis(DDA *);
void dda_set_toolbarvis(DDA *, ddaboolean);

ddaboolean dda_get_statusbarvis(DDA *);
void dda_set_statusbarvis(DDA *, ddaboolean);

ddaboolean dda_get_popupvis(DDA *);
void dda_set_popupvis(DDA *, ddaboolean);

ddaboolean dda_get_tooltipvis(DDA *);
void dda_set_tooltipvis(DDA *, ddaboolean);

int dda_get_menustate(DDA *);
void dda_set_menustate(DDA *, int);

int dda_run(DDA *);

DDA * dda_new();

int ddanalysis(DDA *, FILEPATHS *, GRAPHICS *);

void display(Geometrydata * GData, Analysisdata *, GRAPHICS *gg);


#endif /* _DDA_H_ */