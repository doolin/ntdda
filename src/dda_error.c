
#include <stdlib.h>
#include "dda_error.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _error {

  PrintFunc printer;
  void * stream;
  const char * win_frame_title;
  char * error_string;

};


Error * 
error_new() {

  return (Error*)malloc(sizeof(Error));
}

void
error_delete(Error * e) {
  free(e);
}


/** 
 * register a callback function.
 * 
 * Minimum: need to handle print, fprintf to stderr 
 * and to an error file, and handle MessageBox for 
 * win32.
 * 
 * What we can rely on: 
 * void * to function such as fprintf or MessageBox
 * char * win_frame_title
 * char * error_string
 *
 * So what happens is that the function that gets 
 * registered has to handle all of these.  For 
 * printf or fprintf, the win_frame_title could be 
 * passed in as NULL.
 * 
 * So I need a 3rd function to hook the code-specific  
 * stuff to the callback register.
 */

void 
error_register_handler(Error * e,
		       void * stream,
		       PrintFunc printer,
		       const char * title) {
  e->stream = stream;
  e->printer = printer;
  e->win_frame_title = title;
}

void
error_display(Error * e, const char * message) {

  e->printer(e->stream, e->win_frame_title, message);
} 

#ifdef __cplusplus
}
#endif
