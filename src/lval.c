#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "lval.h"
#include "vm.h"
#include "log.h"
#include "lenv.h"
#include "builtin.h"
#include "tree.h"

lval* lval_num(long x) {
    lval* v = new_lval(LVAL_NUM, 0);
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval* lval_err(char* fmt, ...) {
    lval* v = new_lval(LVAL_ERR, 512);
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);

    v->err = malloc(512);

    vsnprintf(v->err, 511, fmt, va);

    size_t size = strlen(v->err) + 1;
    v->err = realloc(v->err, size);
    resize_lval(v, size);

    va_end(va);

    return v;
}

lval* lval_str(char* s) {
    lval* v = new_lval(LVAL_STR, strlen(s) + 1);
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
    return v;
}

lval* lval_sym(char* s) {
    lval* v = new_lval(LVAL_SYM, strlen(s) + 1);
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

lval* lval_qexpr(void) {
    lval* v = new_lval(LVAL_QEXPR, 0);
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval* lval_sexpr(void) {
    lval* v = new_lval(LVAL_SEXPR, 0);
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval* lval_fun(lbuiltin func) {
    lval* v = new_lval(LVAL_FUN, 0);
    v->type = LVAL_FUN;
    v->builtin = func;
    return v;
}

lval* lval_lambda(lval* formals, lval* body) {
    lval* v = new_lval(LVAL_FUN, 0);
    v->type = LVAL_FUN;

    v->builtin = NULL;

    v->env = lenv_new();

    v->formals = formals;
    v->body = body;
    return v;
}

char* ltype_name(lval_t t) {
    switch(t) {
        case LVAL_FUN: return "Function";
        case LVAL_NUM: return "Number";
        case LVAL_ERR: return "Error";
        case LVAL_SYM: return "Symbol";
        case LVAL_STR: return "String";
        case LVAL_SEXPR: return "S-Expression";
        case LVAL_QEXPR: return "Q-Expression";
        default: return "Unknown";
    }
}

void lval_del(lval* v) {
    switch (v->type) {
        case LVAL_FUN:
            if (!v->builtin) {
                lenv_del(v->env);
                lval_del(v->formals);
                lval_del(v->body);
            }
            break;
        case LVAL_NUM:
            break;
        case LVAL_ERR:
            break;
        case LVAL_STR:
            break;
        case LVAL_SYM:
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
        break;
    }
    del_lval(v);
}

void lval_free(lval* v) {
    switch (v->type) {
        case LVAL_FUN:
            break;
        case LVAL_NUM:
            break;
        case LVAL_ERR:
            free(v->err);
            break;
        case LVAL_STR:
            free(v->str);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            free(v->cell);
        break;
    }
    free(v);
}

lval* lval_call(lenv* e, lval* f, lval* a) {
    if (f->builtin) {
        return f->builtin(e, a);
    }

    int given = a->count;
    int total = f->formals->count;

    while (a->count) {
        if (f->formals->count == 0) {
            lval_del(a);
            return lval_err("Function passed too many arguments. Expected %i, Got %i.", total, given);
        }

        lval* sym = lval_pop(f->formals, 0);

        // Variadic operator
        if (strcmp(sym->sym, "&") == 0){
            if (f->formals->count != 1) {
                lval_del(a);
                return lval_err("Function format invalid. "
                        "Symbol '&' not followed by a single symbol");
            }

            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym);
            lval_del(nsym);
            break;
        }

        lval* val = lval_pop(a, 0);
        lenv_put(f->env, sym, val);
        lval_del(val);
        lval_del(sym);
    }

    lval_del(a);

    // If '&' remains in formal list bind to empty list
    if (f->formals->count > 0 &&
            strcmp(f->formals->cell[0]->sym, "&") == 0) {
        if (f->formals->count != 2) {
            return lval_err("Function format invalid. "
                    "Symbol '&' not followed by a single symbol");
        }

        lval_del(lval_pop(f->formals, 0));

        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();

        lenv_put(f->env, sym, val);
        lval_del(val);
        lval_del(sym);
    }

    if (f->formals->count == 0) {
        f->env->par = e;
        return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    } else {
        return lval_copy(f);
    }
}

int lval_eq(lval* x, lval* y) {
    if (x->type != y->type) { return 0; }

    /* Compare Based upon type */
    switch (x->type) {
        /* Compare Number Value */
        case LVAL_NUM: return (x->num == y->num);

        /* Compare String Values */
        case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
        case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);
        case LVAL_STR: return (strcmp(x->str, y->str) == 0);

        /* If builtin compare, otherwise compare formals and body */
        case LVAL_FUN:
            if (x->builtin || y->builtin) {
              return x->builtin == y->builtin;
            } else {
              return lval_eq(x->formals, y->formals)
                && lval_eq(x->body, y->body);
            }

        /* If list compare every individual element */
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if (x->count != y->count) { return 0; }
            for (int i = 0; i < x->count; i++) {
              /* If any element not equal then whole list not equal */
              if (!lval_eq(x->cell[i], y->cell[i])) { return 0; }
            }
            /* Otherwise lists must be equal */
            return 1;
        break;
    }
    return 0;
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

lval* lval_pop(lval *v, int i) {
    lval* x = v->cell[i];

    memmove(&v->cell[i], &v->cell[i+1],
        sizeof(lval*) * (v->count-i-1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_take(lval *v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval* lval_join(lval* x, lval* y) {
    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }

    lval_del(y);
    return x;
}

lval* lval_copy(lval *v) {
    lval* x;
    gl_log(L_DEBUG, "Copying lval of type: %s", ltype_name(v->type));
    switch (v->type) {
        case LVAL_FUN:
            x = new_lval(v->type, 0);
            if (v->builtin) {
                x->builtin = v->builtin;
            } else {
                x->builtin = NULL;
                x->env = lenv_copy(v->env);
                x->formals = lval_copy(v->formals);
                x->body = lval_copy(v->body);
            }
            break;
        case LVAL_NUM:
            x = new_lval(v->type, 0);
            x->num = v->num;
            break;
        case LVAL_ERR:
            x = new_lval(v->type, strlen(v->err) + 1);
            x->err = malloc(strlen(v->err) + 1);
            strcpy(x->err, v->err);
            break;
        case LVAL_SYM:
            x = new_lval(v->type, strlen(v->sym) + 1);
            x->sym = malloc(strlen(v->sym) + 1);
            strcpy(x->sym, v->sym);
            break;
        case LVAL_STR:
            x = new_lval(v->type, strlen(v->str) + 1);
            x->str = malloc(strlen(v->str) + 1);
            strcpy(x->str, v->str);
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            x = new_lval(v->type, sizeof(lval*) * v->count);
            x->count = v->count;
            x->cell = malloc(sizeof(lval*) * x->count);
            for (int i = 0; i < x->count; i++) {
                x->cell[i] = lval_copy(v->cell[i]);
            }
        break;
    }
    x->type = v->type;
    return x;
}
