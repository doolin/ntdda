


#ifndef _DDA_H_
#define _DDA_H_

#include "geometrydata.h"
#include "analysisdata.h"

/* Need to strip out all windows specific code from 
 * from the numerical engine, if possible.  PEEK defines
 * a PeekMessageLoop for aborting geometry procedures.
 * The code doesn't work anyway.
 */
#define METAFILE 0
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


/* Some standard includes that will most likely 
 * be needed everywhere.
 */
#include<stdio.h>
#include<time.h>





/* Member of DDA struct.  Tracks processor
 * time.
 */
typedef struct _timing_tag {

  /* Total elapsed processor time. */
   double globalstart, globalstop;

}TIMINGINFO;


/* Flags to control what gets saved to a file during an
 * analysis.  Then output can be controlled from the gui,
 * or from flags in an input file.
 */
typedef struct _output_tag {

   int timesteps;

}OUTPUT;



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


#endif /* _INC_DDA */