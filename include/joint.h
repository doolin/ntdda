
#ifndef __JOINT_H__
#define __JOINT_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

typedef struct _joint Joint;


/** @todo Change to incomplete type. */
struct _joint {
  	int	type;
   double epx1, epy1, epx2, epy2;
   double length;
   int is_selected;
};




#ifdef __cplusplus
}
#endif

#endif  /* __JOINT_H__ */