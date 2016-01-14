#ifndef _GL_PRINT_H
#define _GL_PRINT_H

void lval_expr_print(lval* v, char open, char close);
void lval_print(lval* v);
void lval_print_str(lval* v);
void lval_println(lval* v);

#endif
