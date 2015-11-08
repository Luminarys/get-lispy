#include <stdio.h>
#include <stdarg.h>

#include "log.h"

int colored = 1;
log_importance_t loglevel_default = L_ERROR;
log_importance_t v = L_SILENT;

static const char *verbosity_colors[] = {
    [L_SILENT] = "",
    [L_ERROR ] = "\x1B[1;31m",
    [L_INFO  ] = "\x1B[1;34m",
    [L_DEBUG ] = "\x1B[1;30m",
};

void init_log(log_importance_t verbosity) {
    if (verbosity != L_DEBUG) {
        loglevel_default = verbosity;
    }
    v = verbosity;
}

void set_log_level(log_importance_t verbosity) {
    v = verbosity;
}

void gl_log(log_importance_t verbosity, const char* format, ...) {
    if (verbosity <= v) {
        unsigned int c = verbosity;
        /*
        if (c < sizeof(verbosity_colors)/sizeof(char *)) {
            c = sizeof(verbosity_colors)/sizeof(char *) - 1;
        }
        */

        if (colored) {
            fprintf(stderr, "%s", verbosity_colors[c]);
        }

        va_list args;
        va_start (args, format);
        vfprintf(stderr, format, args);
        va_end(args);

        if (colored) {
            fprintf(stderr, "\x1B[0m");
        }

        fprintf(stderr, "\n");
    }
}
