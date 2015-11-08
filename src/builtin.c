#include <string.h>
#include <stdlib.h>

#include "mpc.h"
#include "builtin.h"
#include "loop.h"
#include "lval.h"
#include "read.h"
#include "print.h"
#include "lenv.h"
#include "eval.h"

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k);
    lval_del(v);
}

void lenv_add_builtins(lenv *e) {
  /* List Functions */
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);

    /* Lambdas */
    lenv_add_builtin(e, "~",  builtin_lambda);

    /* Environment manipulation*/
    lenv_add_builtin(e, "def",  builtin_def);
    lenv_add_builtin(e, "=",  builtin_put);

    /* Comparators and other logical operators */
    lenv_add_builtin(e, ">",  builtin_greater_than);
    lenv_add_builtin(e, "<",  builtin_less_than);
    lenv_add_builtin(e, "==",  builtin_equal_to);
    lenv_add_builtin(e, "!=",  builtin_not_equal_to);
    lenv_add_builtin(e, "!",  builtin_not);
    lenv_add_builtin(e, "&&",  builtin_and);
    lenv_add_builtin(e, "||",  builtin_or);
    lenv_add_builtin(e, "if",  builtin_if);

    /* Loading Src */
    lenv_add_builtin(e, "load",  builtin_load);

    /* Print stuff */
    lenv_add_builtin(e, "print",  builtin_print);
    lenv_add_builtin(e, "err",  builtin_print);

    /* Mathematical Functions */
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
}

lval* builtin_op(lenv* e, lval* a, char* op) {
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("Cannot operate on non-number!");
        }
    }

    lval* x = lval_pop(a, 0);

    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->num = -x->num;
    }

    while (a->count > 0) {

        lval* y = lval_pop(a, 0);
        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                lval_del(x);
                lval_del(y);
                x = lval_err("Division By Zero!");
                break;
            }
            x->num /= y->num;
        }
        lval_del(y);
    }
    lval_del(a);
    return x;
}

lval* builtin_load(lenv* e, lval* a) {
    LASSERT_ARG_NUM("load", a, 1);
    LASSERT_TYPE("load", a, 0, LVAL_STR);

    mpc_result_t r;
    if (mpc_parse_contents(a->cell[0]->str, Lispy, &r)) {
        lval* expr = lval_read(r.output, e);
        mpc_ast_delete(r.output);

        while (expr->count) {
            lval* x = lval_eval(e, lval_pop(expr, 0));
            if (x->type == LVAL_ERR) {
                lval_println(x);
            }
            lval_del(x);
        }
        lval_del(expr);
        lval_del(a);

        return lval_sexpr();
    } else {
        char* err_msg = mpc_err_string(r.error);
        mpc_err_delete(r.error);

        lval* err = lval_err("Could not load Library %s", err_msg);
        free(err_msg);
        lval_del(a);

        return err;
    }
}

lval* builtin_head(lenv* e, lval* a) {
    LASSERT_ARG_NUM("head", a, 1);
    LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
    LASSERT_NOT_EMPTY("head", a, 0);

    lval* v = lval_take(a, 0);

    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }
    return v;
}

lval* builtin_tail(lenv* e, lval* a) {
    LASSERT_ARG_NUM("tail", a, 1);
    LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
    LASSERT_NOT_EMPTY("tail", a, 0);

    lval* v = lval_take(a, 0);

    lval_del(lval_pop(v, 0));
    return v;
}

lval* builtin_list(lenv* e, lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_join(lenv* e, lval* a) {
    for (int i = 0;i < a->count; i++) {
        LASSERT_TYPE("tail", a, i, LVAL_QEXPR);
    }

    lval* x = lval_pop(a, 0);

    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);
    return x;
}

lval* builtin_eval(lenv* e, lval* a) {
    LASSERT_ARG_NUM("eval", a, 1);
    LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval* builtin_greater_than(lenv* e, lval* a) {
    LASSERT_ARG_NUM(">", a, 2);
    LASSERT_TYPE(">", a, 0, LVAL_NUM);
    LASSERT_TYPE(">", a, 1, LVAL_NUM);

    lval* x = lval_pop(a, 0);
    lval* y = lval_pop(a, 0);

    x->num = (x->num > y->num) ? 1 : 0;
    lval_del(y);
    lval_del(a);
    return x;
}

lval* builtin_less_than(lenv* e, lval* a) {
    LASSERT_ARG_NUM("<", a, 2);
    LASSERT_TYPE("<", a, 0, LVAL_NUM);
    LASSERT_TYPE("<", a, 1, LVAL_NUM);

    lval* x = lval_pop(a, 0);
    lval* y = lval_pop(a, 0);

    x->num = (x->num < y->num) ? 1 : 0;
    lval_del(y);
    lval_del(a);
    return x;
}

lval* builtin_equal_to(lenv* e, lval* a) {
    LASSERT_ARG_NUM("==", a, 2);

    lval* x = lval_pop(a, 0);
    lval* y = lval_pop(a, 0);

    int r = lval_eq(x, y);
    lval_del(x);
    lval_del(y);
    lval_del(a);
    return lval_num(r);
}

lval* builtin_not_equal_to(lenv* e, lval* a) {
    LASSERT_ARG_NUM("!=", a, 2);

    lval* x = lval_pop(a, 0);
    lval* y = lval_pop(a, 0);

    int r = (0 == lval_eq(x, y)) ? 1 : 0;
    lval_del(x);
    lval_del(y);
    lval_del(a);
    return lval_num(r);
}

lval* builtin_not(lenv* e, lval* a) {
    LASSERT_ARG_NUM("!", a, 1);
    LASSERT_TYPE("!", a, 0, LVAL_NUM);

    lval* x = lval_pop(a, 0);
    x->num = x->num != 0 ? 0 : 1;

    lval_del(a);
    return x;
}

lval* builtin_and(lenv* e, lval* a) {
    LASSERT_ARG_NUM("&&", a, 2);
    LASSERT_TYPE("&&", a, 0, LVAL_NUM);
    LASSERT_TYPE("&&", a, 1, LVAL_NUM);

    lval* x = lval_pop(a, 0);
    lval* y = lval_pop(a, 0);

    x->num = (x->num && y->num);
    lval_del(y);
    lval_del(a);
    return x;
}

lval* builtin_or(lenv* e, lval* a) {
    LASSERT_ARG_NUM("||", a, 2);
    LASSERT_TYPE("||", a, 0, LVAL_NUM);
    LASSERT_TYPE("||", a, 1, LVAL_NUM);

    lval* x = lval_pop(a, 0);
    lval* y = lval_pop(a, 0);

    x->num = (x->num || y->num);
    lval_del(y);
    lval_del(a);
    return x;
}

lval* builtin_if(lenv* e, lval* a) {
    LASSERT_ARG_NUM("if", a, 3);
    for (int i = 0; i < a->count; i++) {
        LASSERT_TYPE("if", a, i, LVAL_QEXPR);
    }

    lval* cond = lval_pop(a, 0);
    lval* on_true = lval_pop(a, 0);
    lval* on_false = lval_pop(a, 0);

    cond->type = LVAL_SEXPR;
    lval* res = lval_eval(e, cond);
    LASSERT(res, res->type == LVAL_NUM,
            "Function if was passed a condition that returned an incorrect type of value. Expected %s, got %s.",
            ltype_name(LVAL_NUM), ltype_name(res->type));
    if (res->num != 0) {
        lval_del(on_false);
        lval_del(a);
        on_true->type = LVAL_SEXPR;
        return lval_eval(e, on_true);
    } else {
        lval_del(on_true);
        lval_del(a);
        on_false->type = LVAL_SEXPR;
        return lval_eval(e, on_false);
    }
}

lval* builtin_var(lenv* e, lval* a, char* func) {
    //LASSERT_TYPE(func, a, 0, LVAL_QEXPR);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function '%s' was passed an incorrect type for argument %i. Expected %s, Got %s.",
            func, 0, ltype_name(LVAL_QEXPR), ltype_name(a->cell[0]->type));
    lval* syms = a->cell[0];

    for (int i = 0; i< syms->count; i++) {
        LASSERT(a, syms->cell[i]->type == LVAL_SYM,
                "Function '%s' cannot define a non-symbol"
                "Expected %s, Got %s.",
                func, ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
    }

    LASSERT(a, syms->count == a->count-1,
            "Function '%s' passed too many arguments for symbols. "
            "Expected %i, Got %i.", func, syms->count, a->count-1);
    for (int i = 0; i< syms->count; i++) {
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i+1]);
        }

        if (strcmp(func, "=") == 0) {
            lenv_put(e, syms->cell[i], a->cell[i+1]);
        }
    }

    lval_del(a);
    return lval_sexpr();
}

lval* builtin_lambda(lenv* e, lval* a) {
    LASSERT_ARG_NUM("~", a, 2);
    LASSERT_TYPE("~", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("~", a, 1, LVAL_QEXPR);

    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
            "Cannot define non-symbol. Expected %s, got %s.",
            ltype_name(LVAL_SYM), ltype_name(a->cell[0]->cell[i]->type));
    }

    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);

}

lval* builtin_print(lenv* e, lval* a) {
    for (int i = 0; i < a->count; i++) {
        lval_print(a->cell[i]);
        putchar(' ');
    }

    putchar('\n');
    lval_del(a);

    return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
    LASSERT_ARG_NUM("error", a, 1);
    LASSERT_TYPE("error", a, 0, LVAL_STR);

    lval* err = lval_err(a->cell[0]->str);
    lval_del(a);
    return err;
}

lval* builtin_def(lenv* e, lval* a) {
    return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
    return builtin_var(e, a, "=");
}

lval* builtin_add(lenv* e, lval* a) {
    return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
    return builtin_op(e, a, "-");
}
lval* builtin_mul(lenv* e, lval* a) {
    return builtin_op(e, a, "*");
}
lval* builtin_div(lenv* e, lval* a) {
    return builtin_op(e, a, "/");
}
