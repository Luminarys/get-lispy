#ifndef _GL_LOG_H
#define _GL_LOG_H

typedef enum {
    L_SILENT = 0,
    L_ERROR = 1,
    L_INFO = 2,
    L_DEBUG = 3,
} log_importance_t;

void init_log(log_importance_t verbosity);
void set_log_level(log_importance_t verbosity);
void gl_log(log_importance_t verbosity, const char* format, ...) __attribute__((format(printf,2,3)));

#endif
