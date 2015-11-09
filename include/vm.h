#ifndef _GL_VM_H
#define _GL_VM_H

#include "lval.h"
#include "lenv.h"
#include "tree.h"

typedef enum {
    OBJECT_LVAL,
    OBJECT_LENV
} object_t;

// For RB tree in GC
typedef struct objectList objectList;
typedef struct object object;

// Stack Object
struct object {
    object_t type;

    union {
        lval* v;
        lenv* e;
    };
    size_t extra_mem;
    // For GC
    RB_ENTRY(object) entry_;
    struct object* next;
    unsigned char marked;
};

typedef struct {
    object* firstObject;
} VM;

void init_vm();
void stop_vm();

int object_compare(object *a, object *b);

void check_mem();
void mark_all();
void mark();
void sweep();

void free_all_objects();

lval* new_lval(lval_t t, size_t extra_mem);
void del_lval(lval* v);

lenv* new_lenv();
void del_lenv(lenv* t);

#endif
