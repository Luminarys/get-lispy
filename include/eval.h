#ifndef _GL_EVAl_H
#define _GL_EVAl_H

lval* lval_eval(lenv* e, lval* v);
lval* lval_eval_sexpr(lenv* e, lval* v);

#endif
