


#ifndef __DDATYPES_H__
#define __DDATYPES_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


/** The reason for explicitly declaring a type boolean is to
 * provide for platform independence.  That is,
 * `boolean' is not a defined type in the c language
 * and counting on the compiler to do the right thing
 * is an invitation to disaster.
 */
typedef int ddaboolean;

#ifndef PI
#define PI 3.141592653
#endif


#ifndef FALSE
#define FALSE 0
#endif 
#ifndef TRUE
#define TRUE (!FALSE)
#endif

/* From Kahan probably. */
#define MACHEPS (3.*((4.0/3.0)-1.0)-1.0)


/** Standard typedefs for constructing callbacks. */
#ifndef PRINTFUNC
#define PRINTFUNC
typedef int  (*PrintFunc)(void *, const char *, ...);
#endif

#ifndef DISPLAYFUNC
#define DISPLAYFUNC
typedef void  (*DisplayFunc)(const char * message);
#endif

#ifndef TRANSMAPFUNC
typedef void (*TransMap)(double ** moments, double T[7][7], double x, 
                         double y, int i0);
#define TRANSMAPFUNC
#endif


#ifndef TRANS_APPLY_FUNC
#define TRANS_APPLY_FUNC
typedef void (*TransApply)(double T[][7], double * D, 
                           double * u1, double * u2);
#endif




#undef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))


/* TODO: collect all the enums for the program to right here
 * for the time being.  Later, a more rational plan can be 
 * established for restricting scope.
 */
//typedef enum _FrictionLaw {coulomb, druckerprager, voight} FrictionLaw;

/* TODO: Get rid of all this stuff.  Put it into a common
 * header file for the dialogs boxes and I/O functions.
 */
typedef struct _blockmat BlockMat;
typedef struct _pstress PStress;
typedef struct _ddapoint DDAPoint;
typedef struct _ddaline DDALine;
typedef struct _ddarect DDARect;
//typedef struct _ddarfile DDARFile;  // results file

/* Try to get rid of this enum. */
typedef enum _pointtype {fixed=0,measured,load,hole, seismic} PointType;

typedef struct {
	double x;
	double y;
 int type;
} DPoint;



#if 0
struct _jointmat {

  /* Need a union of structs and an 
   * enum in here to handle the various types
   * of friction laws.
   */
   double fric;
  	double coh;
	double tens;

  	int type;
};
#endif


struct _ddapoint{
   double x,y;
   double u,v;
   double origx,origy;
  /* Use these instead of the enum typedefed above. */
   enum ptype {pfixed = 0, pload, pmeasured, phole, pseisic} ptype;
  /* Get rid of this */
   int type;
   int blocknum;
};


/** Used in geometry reading code to specify a fixed line,
 * could also be used to specify a material line.
 */
struct _ddaline{
	double x1;
	double y1;
   double x2;
   double y2;
   int type;
};


/* FIXME: Should have same member order as MS RECT type */
struct _ddarect{
   double left;
   double top;
   double right;
   double bottom;
};

struct _blockmat{
  	double dens;
  	double wt;
  	double ymod; 
  	double pois; 
   double damping;
  	double iss[3];
  	double ist[3];
  	double ivel[3];
};


struct _pstress {
  /* End points of lines showing magnitude
   * and direction of principle stress.
   */
   double majorx1,majory1,majorx2,majory2;
   double minorx1,minory1,minorx2,minory2;
};

#ifdef __cplusplus
}
#endif

#endif /* __DDATYPES_H__ */
