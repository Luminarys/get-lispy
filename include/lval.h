#ifndef _SCL_LVAL_H
#define _SCL_LVAL_H

#define LASSERT(args, cond, fmt, ...) \
      if (!(cond)) { \
          lval* err = lval_err(fmt, ##__VA_ARGS__); \
          lval_del(args); \
          return err; \
      }

#define LASSERT_TYPE(func, args, index, expect) \
    LASSERT(args, args->cell[index]->type == expect, \
            "Function '%s' was passed an incorret type for argument %i. Expected %s, Got $s." \
            func, index, ltype_name(expect), ltype_name(args->cell[index]->type))

#define LASSERT_ARG_NUM(func, args, num) \
        LASSERT(args, args->count == num, \
        "Function '%s', passed incorrect number of arguments. Expected %i, Got %i,", \
        func, num, args->count)

#define LASSERT_NOT_EMPTY(func, args, index) \
    LASSERT(args, args->cell[index]->count != 0, \
            "Function '%s' passed {} for argument %i.", func, index)

enum {
    LVAL_NUM,
    LVAL_ERR,
    LVAL_FUN,
    LVAL_SYM,
    LVAL_QEXPR,
    LVAL_SEXPR
};

enum {
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
};

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);

struct lval {
    int type;

    /* Basic */
    long num;
    char* err;
    char* sym;

    /* Function */
    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;

    /* Expression */
    int count;
    struct lval** cell;
};

lval* lval_num(long x);
lval* lval_err(char* m, ...);
lval* lval_sym(char* m);
lval* lval_qexpr(void);
lval* lval_sexpr(void);
lval* lval_fun(lbuiltin func);
lval* lval_call(lenv* e, lval* f, lval* a);
lval* lval_lambda(lval* formals, lval* body);

char* ltype_name(int t);

void lval_del(lval* v);
lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* v, lval* x);
lval* lval_copy(lval *v);

#endif
