#include "object.h"


#include <stdlib.h>

struct object {
    struct object_header * oh;
};

void object_delete (void * obj) {
    struct object * o = (struct object *) obj;
    o->oh->delete(obj);
}

void * object_copy (const void * obj) {
    struct object * o = (struct object *) obj;
    return o->oh->copy(obj);
}

int object_cmp (const void * lhs, const void * rhs) {
    struct object * o = (struct object *) lhs;
    return o->oh->cmp(lhs, rhs);
}
