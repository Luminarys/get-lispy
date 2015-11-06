#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

#include "mpc.h"
#include "lval.h"
#include "lenv.h"
#include "builtin.h"
#include "read.h"
#include "eval.h"
#include "print.h"

int main(int argc, char** argv) {
    puts("Lispy version 0.1.1");
    puts("Use Ctrl+c to Exit\n");

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
            " \
            number   : /-?[0-9]+/ ; \
            symbol : /[a-zA-Z0-9~_+\\-*\\/\\\\=<>!&]+/ ; \
            qexpr    : '{' <expr>* '}' ; \
            sexpr    : '(' <expr>* ')' ; \
            expr     : <number> | <symbol> | <sexpr> | <qexpr> ; \
            lispy    : /^/ <expr>* /$/ ; \
            ",
            Number, Symbol, Qexpr, Sexpr, Expr, Lispy);

    lenv* e = lenv_new();
    lenv_add_builtins(e);

    while (1) {
        char* input = readline("get lispy> ");

        add_history(input);

        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lispy, &r)) {
            // WE REPL NOW
            lval* x = lval_eval(e, lval_read(r.output));
            lval_println(x);
            lval_del(x);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Symbol, Qexpr, Sexpr, Expr, Lispy);
    return 0;
}
