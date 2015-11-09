#include "stdlib.h"

#include "vm.h"
#include "lval.h"
#include "lenv.h"
#include "log.h"

size_t mem_in_use = 0;
size_t mem_quota = 200;

const size_t lval_size = sizeof(lval);
const size_t lenv_size = sizeof(lenv);
const size_t obj_size = sizeof(object);

VM* vm;

void init_vm() {
    vm = malloc(sizeof(VM));
    vm->firstObject = NULL;
}

void stop_vm() {
    free_all_objects();
    free(vm);
}

int object_compare(object* a, object* b) {
    if (a > b) {
        return 1;
    }

    return (a == b ? 0 : -1);
}

RB_HEAD(objectList, object) objectListHead = RB_INITIALIZER(&objectListHead);
RB_GENERATE(objectList, object, entry_, object_compare);

lval* new_lval(lval_t t, size_t extra_mem) {
    gl_log(L_DEBUG, "Allocating a new lval(size %zu) of type: %s with additional memory %zu", lval_size, ltype_name(t), extra_mem);
    mem_in_use += lval_size + extra_mem;
    check_mem();
    lval* v = malloc(lval_size);

    object* o = malloc(obj_size);
    o->type = OBJECT_LVAL;
    o->v = v;
    o->extra_mem = extra_mem;
    o->marked = 0;

    o->next = vm->firstObject;
    vm->firstObject = o;
    v->o = o;

    RB_INSERT(objectList, &objectListHead, o);
    //gl_log(L_DEBUG, "Pointer %p has been inserted into the RB tree", o);

    return v;
}

void resize_lval(lval* v, size_t new_extra_mem) {
    mem_in_use -= v->o->extra_mem;
    mem_in_use += new_extra_mem;
    v->o->extra_mem = new_extra_mem;
}

void del_lval(lval* v) {
    gl_log(L_DEBUG, "Deleting a lval of type: %s, object pointer %p", ltype_name(v->type), v->o);
    RB_REMOVE(objectList, &objectListHead, v->o);
}

lenv* new_lenv() {
    gl_log(L_DEBUG, "Allocating a new lenv, size %zu", lenv_size);
    mem_in_use += lenv_size;
    check_mem();
    lenv* e = malloc(lenv_size);

    object* o = malloc(obj_size);
    o->type = OBJECT_LENV;
    o->extra_mem = 0;
    o->e = e;
    o->marked = 0;

    o->next = vm->firstObject;
    vm->firstObject = o;
    e->o = o;

    //gl_log(L_DEBUG, "Pointer %p has been inserted into the RB tree", o);
    RB_INSERT(objectList, &objectListHead, o);

    return e;
}

void del_lenv(lenv* e) {
    gl_log(L_DEBUG, "Deleting a lenv");
    RB_REMOVE(objectList, &objectListHead, e->o);
}

void check_mem() {
    if (mem_in_use >= mem_quota) {
        gl_log(L_INFO, "Mem quota of %zu exceeded, with mem use %zu. Triggering GC.", mem_quota, mem_in_use);
        mark_all();
        sweep();
        mem_quota = 2*mem_in_use;
        gl_log(L_INFO, "GC done, new mem quota set at %zu", mem_quota);
    }
}

void mark(object* o) {
    o->marked = 1;
}

void sweep() {
	object** o = &vm->firstObject;
    while (*o) {
        if (!(*o)->marked) {
            object* unreached = *o;
            *o = unreached->next;
            // Free shit
            if (unreached->type == OBJECT_LVAL) {
                gl_log(L_DEBUG, "Freeing a lval of type %s size %zu, pointer %p", ltype_name(unreached->v->type), unreached->extra_mem + lval_size, unreached);
                mem_in_use -= (lval_size + unreached->extra_mem);
                lval_free(unreached->v);
            } else if (unreached->type == OBJECT_LENV) {
                gl_log(L_DEBUG, "Freeing a lenv of size %zu", lenv_size + unreached->extra_mem);
                mem_in_use -= lenv_size;
                lenv_free(unreached->e);
            }
            free(unreached);
        } else {
            //gl_log(L_DEBUG, "Unmarking object %p", *o);
            (*o)->marked = 0;
            o = &(*o)->next;
        }
    }
}

void mark_all() {
    object* o;
    RB_FOREACH(o, objectList, &objectListHead) {
        //gl_log(L_DEBUG, "Marking object %p", o);
        mark(o);
    }
}

void free_all_objects() {
	object** o = &vm->firstObject;
    while (*o) {
       object* unreached = *o;
       *o = unreached->next;
       if (unreached->type == OBJECT_LVAL) {
           gl_log(L_DEBUG, "Freeing a lval of type: %s", ltype_name(unreached->v->type));
           mem_in_use -= (lval_size + unreached->extra_mem);
           lval_free(unreached->v);
       } else if (unreached->type == OBJECT_LENV) {
           gl_log(L_DEBUG, "Freeing a lenv");
           mem_in_use -= (lenv_size + unreached->extra_mem);
           lenv_free(unreached->e);
       }
       //RB_REMOVE(objectList, &objectListHead, unreached);
       free(unreached);
    }
}
