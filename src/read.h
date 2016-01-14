#ifndef _GL_READ_H
#define _GL_READ_H

#include "mpc.h"
#include "lval.h"

lval* lval_read_str(mpc_ast_t* t);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);

#endif
