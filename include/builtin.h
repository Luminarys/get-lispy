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

lval* builtin_lambda(lenv* e, lval* a);

lval* builtin_load(lenv* e, lval* a);

lval* builtin_print(lenv* e, lval* a);
lval* builtin_err(lenv* e, lval* a);

lval* builtin_greater_than(lenv* e, lval* a);
lval* builtin_less_than(lenv* e, lval* a);
lval* builtin_equal_to(lenv* e, lval* a);
lval* builtin_not(lenv* e, lval* a);
lval* builtin_and(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);
lval* builtin_not_equal_to(lenv* e, lval* a);
lval* builtin_if(lenv* e, lval* a);

lval* builtin_var(lenv* e, lval* a, char* func);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);

#endif
