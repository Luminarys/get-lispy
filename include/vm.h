#ifndef _GL_VM_H
#define _GL_VM_H

#include "lval.h"
#include "lenv.h"
#include "list.h"

typedef enum {
    OBJECT_LVAL,
    OBJECT_LENV
} object_t;

// Stack Object
typedef struct object {
    object_t type;

    union {
        lval* v;
        lenv* e;
    };
    size_t extra_mem;
    // For GC
    struct object* next;
    unsigned char marked;
} object;

typedef struct {
    object* firstObject;
    list_t* allocated_objects;
} VM;

void init_vm();
void stop_vm();

void check_mem();
void mark_all();
void mark();
void sweep();

void free_all_objects();

lval* new_lval(lval_t t, size_t extra_mem);
void del_lval(lval* v);

lenv* new_lenv();
void del_lenv(lenv* t);

int find_object(list_t *list, object* o);

#endif
