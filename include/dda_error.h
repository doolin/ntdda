
#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _error Error;

typedef void  (*Print_Func)(void *, const char *, const char *);

Error * error_new();
void error_delete(Error *);

void error_register_handler(Error * e, void * stream, Print_Func callback, const char * title);

void error_display(Error * e, const char * message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __ERROR_H__ */
