#include "object.h"


#include <stdlib.h>

struct object_header {
    struct object * o;
};

void object_delete (void * obj) {
    struct object_header * oh = (struct object_header *) obj;
    oh->o->delete(obj);
}

void * object_copy (const void * obj) {
    struct object_header * oh = (struct object_header *) obj;
    return oh->o->copy(obj);
}

int object_cmp (const void * lhs, const void * rhs) {
    struct object_header * oh = (struct object_header *) lhs;
    return oh->o->cmp(lhs, rhs);
}
