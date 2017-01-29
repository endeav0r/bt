#include "object.h"

#include "container/tags.h"

#include <stdlib.h>

void object_init (void * obj, const struct object_vtable * vtable) {
    struct object_header * oh = (struct object_header *) obj;
    oh->vtable = vtable;
    oh->tags = NULL;
}

void object_delete (void * obj) {
    struct object_header * oh = (struct object_header *) obj;
    if (oh->tags != NULL)
        ODEL(oh->tags);
    oh->vtable->delete(obj);
}

void * object_copy (const void * obj) {
    struct object_header * oh = (struct object_header *) obj;
    struct object_header * copy = oh->vtable->copy(obj);
    if (oh->tags != NULL) {
        copy->tags = OCOPY(oh->tags);
    }
    return copy;
}

int object_cmp (const void * lhs, const void * rhs) {
    struct object_header * oh = (struct object_header *) lhs;
    return oh->vtable->cmp(lhs, rhs);
}

struct tags * object_tags (void * obj) {
    struct object_header * oh = (struct object_header *) obj;
    if (oh->tags == NULL)
        oh->tags = tags_create();
    return oh->tags;
}
