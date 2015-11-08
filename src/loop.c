#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

#include "loop.h"
#include "log.h"
#include "mpc.h"
#include "lval.h"
#include "list.h"
#include "vm.h"
#include "lenv.h"
#include "builtin.h"
#include "read.h"
#include "eval.h"
#include "print.h"

int main(int argc, char** argv) {
    init_log(L_DEBUG);
    gl_log(L_DEBUG, "Log initialized!");
    init_vm();
    puts("Lispy version 0.1.1");
    puts("Use Ctrl+c to Exit\n");

    Number = mpc_new("number");
    Symbol = mpc_new("symbol");
    String = mpc_new("string");
    Comment = mpc_new("comment");
    Qexpr = mpc_new("qexpr");
    Sexpr = mpc_new("sexpr");
    Expr = mpc_new("expr");
    Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
            "                                                    \
            number   : /-?[0-9]+/ ;                              \
            symbol   : /[a-zA-Z0-9~_+\\-*\\/\\\\=<>!&|]+/ ;      \
            string   : /\"(\\\\.|[^\"])*\"/ ;                    \
            comment : /;[^\\r\\n]*/ ;                            \
            qexpr    : '{' <expr>* '}' ;                         \
            sexpr    : '(' <expr>* ')' ;                         \
            expr     : <number> | <string> | <symbol>            \
                       | <comment> | <sexpr> | <qexpr> ;         \
            lispy    : /^/ <expr>* /$/ ;                         \
            ",
            Number, Symbol, String, Comment, Qexpr, Sexpr, Expr, Lispy);

    lenv* e = lenv_new();
    lenv_add_builtins(e);

    if (argc >= 2) {
        for (int i = 1; i< argc; i++) {
            lval* args= lval_add(lval_sexpr(), lval_str(argv[i]));
            lval* x = builtin_load(e, args);
            if (x->type == LVAL_ERR) {
                lval_println(x);
            } else {
                gl_log(L_DEBUG, "Loaded in library %s!", argv[i]);
            }
            lval_del(x);
        }
    }

    while (1) {
        char* input = readline("get lispy> ");

        add_history(input);

        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lispy, &r)) {
            // WE REPL NOW
            lval *i = lval_read(r.output);
            if (i && i->cell[0] && i->cell[0]->type==LVAL_SYM && strcmp(i->cell[0]->sym, "quit") == 0) {
                lval_del(i);
                mpc_ast_delete(r.output);
                free(input);
                break;
            }
            lval* x = lval_eval(e, i);
            lval_println(x);

            lval_del(i);
            lval_del(x);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    stop_vm();
    mpc_cleanup(8, Number, Symbol, String, Comment, Qexpr, Sexpr, Expr, Lispy);
    return 0;
}
