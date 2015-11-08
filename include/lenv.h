#ifndef _GL_LENV_H
#define _GL_LENV_H

#include "lval.h"

struct lenv {
    lenv* par;
    int count;
    char** syms;
    lval** vals;
};

lenv* lenv_new(void);

void lenv_del(lenv* e);
void lenv_free(lenv* e);

lval* lenv_get(lenv* e, lval* k);
void lenv_def(lenv* e, lval* k, lval* v);
void lenv_put(lenv* e, lval* k, lval* v);
lenv* lenv_copy(lenv* e);
#endif
