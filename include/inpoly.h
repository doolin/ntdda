

#ifndef __INPOLY_H__
#define __INPOLY_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


int pointinpoly  (int blocknumber,
                  double x11,
                  double y11,
                  int ** vindex,
                  double ** vertices);



int inpoly        (double ** poly,
                   int npoints,
                   double xt,
                   double yt);


#ifdef __cplusplus
}
#endif

#endif  /* __INPOLY_H__ */
