#ifndef _SCL_BUILTIN_H
#define _SCL_BUILTIN_H

#include "lval.h"
#include "lenv.h"

void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);

lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);


lval* builtin_op(lenv* e, lval* a, char* op);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);

#endif
