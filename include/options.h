#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


/* FIXME: Get rid of this contraption, move everything
 * to bit fields for handling boolean values.
 */
typedef struct _options_tag Options;


struct _options_tag {
  /* Booleans for controlling output.  These probably need to 
   * into a different struct.  These need to go into a project
   * output struct.
   */
   int spyplots;
   int flagstates;
   int maxdisp;
   int maxtstep;
   int timing;
   int parameters;
   int fixedpoints;
   int measuredpoints;
   int blockareas;

};


#ifdef __cplusplus
}
#endif


#endif  /* __OPTIONS_H__ */