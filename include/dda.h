


#ifndef __DDA_H__
#define __DDA_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif



#include "ddatypes.h"
#include "geometrydata.h"

#include "analysisdata.h"



/** Find a way to get rid of this. */
#include "graphics.h"





/* The dreaded RPC_ASYNC_STATE warning. */
/* Once the headers are fixed, this can go to 
 * a warn once instead of a disable.
 */
#ifdef WIN32
#pragma warning( disable : 4115 )        
#endif


/* These defs provide for some
 * self-documentation in the code.
 */
#define STATIC 0
#define DYNAMIC 1



/* TODO: header file for functions that are common to 
 * both the Win32 code and the back end code.
 */
//void error(char *);




/* FIXME: move this into a better header. */

typedef struct gui_tag Gui;


typedef struct progbar_tag ProgressBarInfo;





/* Pass this around instead of using globals.
 */
typedef struct _dda_tag DDA;
typedef struct _dda_gui DDAGui;


int ddanalysis(DDA *, Filepaths *);
void display(Geometrydata * GData, Analysisdata *);





typedef void (*SetToggleFunc)(DDA *, ddaboolean);
typedef int  (*GetToggleFunc)(DDA *);


DDA *          dda_new                (void);
void           dda_delete             (DDA *);

int            dda_run(DDA *);

Geometrydata * dda_get_geometrydata   (DDA *);
void           dda_set_geometrydata   (DDA *, 
                                       Geometrydata * geometrydata);

Analysisdata * dda_get_analysisdata   (DDA *);
void           dda_set_analysisdata   (DDA *, 
                                       Analysisdata * analysisdata);

ddaboolean     dda_get_toolbarvis     (DDA *);
void           dda_set_toolbarvis     (DDA *, 
                                       ddaboolean);

ddaboolean     dda_get_statusbarvis   (DDA *);
void           dda_set_statusbarvis   (DDA *, 
                                       ddaboolean);

ddaboolean     dda_get_popupvis       (DDA *);
void           dda_set_popupvis       (DDA *, 
                                       ddaboolean);

ddaboolean     dda_get_tooltipvis     (DDA *);
void           dda_set_tooltipvis     (DDA *, 
                                       ddaboolean);

int            dda_get_menu_state     (DDA *);
void           dda_set_menu_state     (DDA *, int);

int            dda_get_menu_state     (DDA * dda);
void           dda_set_menu_state     (DDA * dda, 
                                       int state);


int            dda_get_xcur           (DDA * dda);
void           dda_set_xcur           (DDA * dda, 
                                       int xcur);

int            dda_get_ycur           (DDA * dda);
void           dda_set_ycur           (DDA * dda,  
                                       int ycur);


void           dda_set_output_directory(const char * dirname,
                                        size_t dirnamesize);

/** Some platform independent prototypes. */
void           dda_display_error      (const char * message);

void           dda_display_warning    (const char * message);

void           dda_display_info       (const char * message);

#ifdef __cplusplus
}
#endif

#endif /* __DDA_H__ */

