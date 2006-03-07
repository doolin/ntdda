/** Everything to do with analysis parameters.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "constants.h"

#ifdef __cpluplus
extern "C" {
#endif
#if 0
}
#endif


/* Various parameters used during an analysis.
 * All of these appear to be constants set at the 
 * beginning of an analysis, and not changed during
 * runtime.
 */
struct _constants {

   double openclose;    /*  s0  */
   double opencriteria; /*  f0  */
   double norm_spring_pen; /* g3  */

//#if 0
   double w0;  /* related to the physical scale of the model. */
   double norm_extern_dist; /* d0 */
   double norm_pen_dist;    /* d9  */
//#endif
   double angle_olap;      /*  h1  */
   double shear_norm_ratio;  /* h2 */
  /* new, used in contact determination to 
   * prevent division by 0 (in proj()) 
   */
   double min_refline_factor;  
};



Constants *
constants_new(void) {

   Constants * constants;

   constants = (Constants *)malloc(sizeof(Constants));
#if _DEBUG
   memset((void*)constants,0xda,(size_t)sizeof(Constants));
#endif
   return constants;

}

Constants *
constants_new_defaults(void) {

   Constants * constants;

   constants = (Constants *)malloc(sizeof(Constants));
#if _DEBUG
   memset((void*)constants,0xda,(size_t)sizeof(Constants));
#endif

   constants_set_defaults(constants);
   return constants;

}






void

constants_validate(Constants * c) {





}




void
constants_delete(Constants * c) {

   memset(c,0xdd,sizeof(Constants));
   free(c);
}


Constants *
constants_clone(Constants *ci) {

   Constants * co = constants_new();
   memcpy(co,ci,sizeof(Constants));
   return co;
}


void
constants_set_w0(Constants * c, double w0) {

   c->w0 = w0;

}



double 
constants_get_w0(Constants * c) {

   return c->w0;
}




double      
constants_get_shear_norm_ratio (Constants * c) {

   return c->shear_norm_ratio;
}


void        
constants_set_shear_norm_ratio (Constants * c, double shear_norm_ratio) {

   c->shear_norm_ratio = shear_norm_ratio;
}



// was s0, hardwired to 0.0002
double      constants_get_openclose         (Constants * c) {

    return c->openclose;
}

void        constants_set_openclose         (Constants * c,
                                             double openclose) {

   c->openclose = openclose;
}


double      
constants_get_opencriteria (Constants * c) {

   return c->opencriteria;
}


void        
constants_set_opencriteria (Constants * c, double opencriteria) {

   c->opencriteria = opencriteria;
}


double      
constants_get_norm_spring_pen (Constants * c) {

   return c->norm_spring_pen;
}

void        
constants_set_norm_spring_pen (Constants * c, double norm_spring_pen) {

   c->norm_spring_pen = norm_spring_pen;
}


double   
constants_get_angle_olap (Constants * c) {

   return c->angle_olap;
}

void        
constants_set_angle_olap (Constants * c, double angle_olap) {

   c->angle_olap = angle_olap;
}


double      
constants_get_min_refline_factor(Constants * c) {

   return c->min_refline_factor;
}


void        
constants_set_min_refline_factor(Constants * c, double min_refline_factor) {

    c->min_refline_factor = min_refline_factor;
}


double
constants_get_norm_extern_dist(Constants * c) {

   return c->norm_extern_dist;
}


double
constants_get_norm_pen_dist(Constants * c) {

   return c->norm_extern_dist;
}




void
constants_display_warning(const char * warning) {

}



/** 
 * @todo  Look in the GHS code from 1994 and 
 * hardwire the default values of the constants
 * in this function.  For "emergency use only".
 */
void
constants_set_defaults(Constants * c) {

   c->openclose =  .0002;  /* s0 = .0002  */
   c->opencriteria = .0000002;  /* f0 = .0000002;  */
   c->norm_spring_pen = .0004;//g3 = .0004;   /* g3 = .0004;  */
   c->angle_olap = 3;   /* h1 = 3; */
   c->shear_norm_ratio = 2.5;  /* h2 = 2.5; */   
}



/** 
 * @todo Change behavior of this function to throw an error
 *       if a NULL Constants * is passed in.
 */   
void 
constants_init(Constants * constants, double maxdisplacement) {

      
   double g3;  /* normal spring penetration ratio */
   double d0;
   double d9;
   //int m4, m5;
  /* m4: step    of graphic output                  */
  /* m5: number  of graphic output                  */
  /* m4 and m5 are related to the grf1 file, which is not currently 
   * being compiled to use.
   */
	//m4 = (int) floor(ad->nTimeSteps/20.0-.0001)+1;
	//m5 = (int) floor(ad->nTimeSteps/m4  -.0001)+1;
   //ad->m4 = m4;  


  /* (GHS: g4 save g0 for fixed points and 0 contacts)     */
   //ad->g4=ad->g0;
   //ad->g4 = ad->contactpenalty;  
  /* This appears to be the only place FPointSpring 
   * is set.  Which implies that the contact penalty
   * has already been initializationed.  Where?
   */
  /* FIXME: This is the wrong place to initialize this value. */
   //ad->FPointSpring = ad->contactpenalty;

   assert(constants != NULL);

   //if (constants == NULL) {

      //ad->constants = (CONSTANTS *)malloc(sizeof(CONSTANTS));
      //memset((void*)ad->constants,0xda,(size_t)sizeof(CONSTANTS));

      //constants = constants_new();

     /* The call to cons() just sets the value of some 
      * constants that would be better set in this 
      * procedure, or allowed to be set by the user.
      * So all this code has now been moved from cons()
      * which is not ever called again.
      * FIXME: Move all this stuff somewhere else and 
      * implement it as an option.
      */
     /* pre-defined changeble constants                */
     /* s0 : criteria of open-close                    */
     /* f0 : criteria of opening                       */
     /* g3 : allowable normal spring penetration ratio */
     /* d0 : normal external    distance for contact   */
     /* d9 : normal penetration distance for contact   */
     /* h1 : angle overlapping degrees   for contact   */
     /* h2 : ratio for shear spring versus normal one  */
      //constants->openclose =  .0002;  /* s0 = .0002  */
      //constants->opencriteria = .0000002;  /* f0 = .0000002;  */
      //g3 = constants->norm_spring_pen = .0004;
      //g3 = .0004;   /* g3 = .0004;  */
      //constants->angle_olap = 3;   /* h1 = 3; */
      //constants->shear_norm_ratio = 2.5;  /* h2 = 2.5; */   
   //} 
   //else
   //{ 
      //char mess[80];
      //sprintf(mess,"g3: %f, w0: %f",g3, ad->constants->w0);
      //iface->displaymessage(mess);
   //}

  /* g2 is described in the manual as the maximum allowable 
   *  displacement ratio, recommended between 0.001-0.01.
   */
   //ad->constants->norm_extern_dist = 2.5*(ad->constants->w0)*(ad->g2);
   //d0 = 2.5*(ad->constants->w0)*(ad->g2);
   //d0 = 2.5*(ad->constants->w0)*(ad->maxdisplacement);
  /* d0 might be 2\rho on p. 142, GHS 1988 with corresponding
   * Figure 4.2 on p. 143 (p. 32, Fig. 3.2, p. 55, GHS tech note).
   */  
  /* FIXME: Change the 2.5 to 2.0, since it is supposed to be 
   * 2*\rho anyway.  If this blows everything to pieces, find
   * out where the problem is and fix it there.
   */
   d0 = constants->norm_extern_dist = 2.5*(constants->w0)*(maxdisplacement);
   //d0 = ad->constants->norm_extern_dist = 2.0*(ad->constants->w0)*(ad->maxdisplacement);
      
   g3 = constants->norm_spring_pen;

   d9 = constants->norm_pen_dist = 0.3*d0;

   if (d9 < 3.0*g3*(constants->w0))  {
      constants->norm_pen_dist = d9 = 3.0*g3*(constants->w0);
   }

   if (d0 < d9) {

      constants->norm_extern_dist = d0 = d9;
      constants_display_warning("d0 < d9 in constants_init()");
   }

}  


/**
 * @todo  Change the call to pass in the level of indentation
 * as a const char * so these #defines can go away.
 */
#define I1 "   "
#define I2 "      "

void 
constants_print_xml(Constants * constants, PrintFunc printer, void * stream) {

   printer(stream,I1"<AConstants>\n"); 	 
   printer(stream,I2"<Openclose value=\"%f\"/>\n",constants->openclose ); 	    
   printer(stream,I2"<Opencriteria value=\"%.10f\"/>\n",constants->opencriteria); 	    
   printer(stream,I2"<NormSpringPen value=\"%f\"/>\n",constants->norm_spring_pen); 	    
   printer(stream,I2"<AngleOverlap value=\"%f\"/>\n",constants->angle_olap); 	    
   printer(stream,I2"<ShearNormRatio value=\"%f\"/>\n",constants->shear_norm_ratio);
   printer(stream,I1"</AConstants>\n\n"); 
   /*
   openclose =  .0002;  /* s0 = .0002  
   opencriteria = .0000002;  /* f0 = .0000002;  
   norm_spring_pen = .0004;//g3 = .0004;   /* g3 = .0004;  
   angle_olap = 3;   /* h1 = 3; 
   shear_norm_ratio = 2.5;  /* h2 = 2.5; 
   */   
}

  

/**
 * @todo Change printed output to match a matlab struct.
 */
void 
constants_print_matlab(Constants * c, PrintFunc printer, void * stream) {


   //if(ad->cts == 1)
   //{
   printer(stream,"# Analysis parameters\n");
   printer(stream,"# openclose opencriteria    w0     norm_spring_pen"
                            " norm_extern_dist norm_pen_dist angle_olap"
                            " shear_norm_ratio\n");
   //}

   printer(stream,"  %8.5f %10.5f %10.5f     %8.5f   %10.5f %10.5f %10.5f %10.5f\n",
                       c->openclose           /* s0 */, 
                       c->opencriteria        /* f0 */,
                       c->w0                  /* related to the physical scale of the model. */,
                       c->norm_spring_pen     /* g3  */,
                       c->norm_extern_dist    /* d0 */,
                       c->norm_pen_dist       /* d9  */,
                       c->angle_olap          /*  h1  */,
                       c->shear_norm_ratio    /* h2 */);

}





#ifdef __cplusplus
}
#endif



