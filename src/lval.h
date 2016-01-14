#ifndef _GL_LVAL_H
#define _GL_LVAL_H

#define LASSERT(args, cond, fmt, ...) \
      if (!(cond)) { \
          lval* err = lval_err(fmt, ##__VA_ARGS__); \
          lval_del(args); \
          return err; \
      }

#define LASSERT_TYPE(func, args, index, expect) \
    LASSERT(args, args->cell[index]->type == expect, \
            "Function '%s' was passed an incorrect type for argument %d. Expected %s, got %s.", \
            func, index, ltype_name(expect), ltype_name(args->cell[index]->type))

#define LASSERT_ARG_NUM(func, args, num) \
        LASSERT(args, args->count == num, \
        "Function '%s', passed incorrect number of arguments. Expected %i, Got %i.", \
        func, num, args->count)

#define LASSERT_NOT_EMPTY(func, args, index) \
    LASSERT(args, args->cell[index]->count != 0, \
            "Function '%s' passed {} for argument %i.", func, index)

typedef enum {
    LVAL_ERR,
    LVAL_FUN,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_SEXPR,
    LVAL_STR,
    LVAL_QEXPR
} lval_t;

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
    lval_t type;

    /* Basic */
    long num;
    char* err;
    char* sym;
    char* str;

    /* Function */
    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;

    /* Expression */
    int count;
    struct lval** cell;

    /* Parent Object */
    struct object* o;
};

lval* lval_num(long x);
lval* lval_err(char* m, ...);
lval* lval_sym(char* m);
lval* lval_str(char* m);
lval* lval_qexpr(void);
lval* lval_sexpr(void);
lval* lval_fun(lbuiltin func);
lval* lval_call(lenv* e, lval* f, lval* a);
lval* lval_lambda(lval* formals, lval* body);

char* ltype_name(lval_t t);

int lval_eq(lval* x, lval* y);

void lval_del(lval* v);
void lval_free(lval* v);

lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* v, lval* x);
lval* lval_copy(lval *v);

#endif
