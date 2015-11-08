#include "stdlib.h"

#include "vm.h"
#include "lval.h"
#include "lenv.h"
#include "log.h"
#include "list.h"

size_t mem_in_use = 0;
size_t mem_quota = 100;

const size_t lval_size = sizeof(lval);
const size_t lenv_size = sizeof(lenv);
const size_t obj_size = sizeof(object);

VM* vm;

void init_vm() {
    vm = malloc(sizeof(VM));
    vm->firstObject = NULL;
    vm->allocated_objects = create_list();
}

void stop_vm() {
    free_all_objects();
    list_free(vm->allocated_objects);
    free(vm);
}

lval* new_lval(lval_t t, size_t extra_mem) {
    gl_log(L_DEBUG, "Allocating a new lval of type: %s with additional memory %zu", ltype_name(t), extra_mem);
    mem_in_use += lval_size + extra_mem;
    check_mem();
    lval* v = malloc(lval_size);

    object* o = malloc(obj_size);
    o->type = OBJECT_LVAL;
    o->v = v;
    o->extra_mem = extra_mem;

    o->next = vm->firstObject;
    vm->firstObject = o;

    list_add(vm->allocated_objects, o);

    return v;
}

void del_lval(lval* v) {
    gl_log(L_DEBUG, "Deleting a lval of type: %s", ltype_name(v->type));

    object* o = malloc(obj_size);
    o->type = OBJECT_LVAL;
    o->v = v;

    int i = find_object(vm->allocated_objects, o);
    list_del(vm->allocated_objects, i);

    free(o);
}

lenv* new_lenv() {
    mem_in_use += lenv_size;
    check_mem();
    lenv* e = malloc(lenv_size);

    object* o = malloc(obj_size);
    o->type = OBJECT_LENV;
    o->e = e;

    o->next = vm->firstObject;
    vm->firstObject = o;

    list_add(vm->allocated_objects, o);

    return e;
}

void del_lenv(lenv* e) {
    gl_log(L_DEBUG, "Deleting a lenv");
    object* o = malloc(obj_size);
    o->type = OBJECT_LENV;
    o->e = e;

    int i = find_object(vm->allocated_objects, o);
    list_del(vm->allocated_objects, i);

    free(o);
}

void check_mem() {
    if (mem_in_use >= mem_quota) {
        gl_log(L_INFO, "Mem quota exceeded, triggering GC.");
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
                gl_log(L_DEBUG, "Freeing a lval of type: %s", ltype_name(unreached->v->type));
                mem_in_use -= (lval_size + unreached->extra_mem);
                lval_free(unreached->v);
            } else if (unreached->type == OBJECT_LENV) {
                gl_log(L_DEBUG, "Freeing a lenv");
                mem_in_use -= lenv_size;
                lenv_free(unreached->e);
            }
            free(unreached);
        } else {
            (*o)->marked = 0;
            o = &(*o)->next;
        }
    }
}

void mark_all() {
  	for (int i = 0; i < vm->allocated_objects->length; i++) {
  	  mark(vm->allocated_objects->items[i]);
  	}
}

void free_all_objects() {
	object** o = &vm->firstObject;
    while (*o) {
       object* unreached = *o;
       *o = unreached->next;
       if (unreached->type == OBJECT_LVAL) {
           gl_log(L_DEBUG, "Freeing a lval of type: %s", ltype_name(unreached->v->type));
           mem_in_use -= lval_size;
           lval_free(unreached->v);
       } else if (unreached->type == OBJECT_LENV) {
           gl_log(L_DEBUG, "Freeing a lenv");
           mem_in_use -= lenv_size;
           lenv_free(unreached->e);
       }
       free(unreached);
    }
}

int find_object(list_t *list, object* o) {
    for (int i = 0; i < list->length; ++i) {
        if (o->type == OBJECT_LVAL) {
            if (((object *)list->items[i])->type != o->type) {
                continue;
            }
            if (((object *)list->items[i])->v == o->v) {
                return i;
            }
        } else if (o->type == OBJECT_LENV) {
            if (((object *)list->items[i])->type != o->type) {
                continue;
            }
            if (((object *)list->items[i])->e == o->e) {
                return i;
            }
        }
    }
    return -1;
}
