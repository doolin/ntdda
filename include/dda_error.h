
#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _error Error;

#ifndef PRINTFUNC
#define PRINTFUNC
typedef int  (*PrintFunc)(void *, const char *, const char *);
#endif

Error * error_new();
void error_delete(Error *);

void error_register_handler(Error * e, void * stream, PrintFunc callback, const char * title);

void error_display(Error * e, const char * message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __ERROR_H__ */
