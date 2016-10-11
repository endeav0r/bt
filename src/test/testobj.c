#include "testobj.h"

#include <stdlib.h>


const struct object testobj_object = {
    (void (*) (void *)) testobj_delete,
    (void * (*) (const void *)) testobj_copy,
    (int (*) (const void *, const void *)) testobj_cmp
};


struct testobj * testobj_create (unsigned int value) {
    struct testobj * testobj = malloc(sizeof(struct testobj));
    testobj->object = &testobj_object;
    testobj->value = value;
    return testobj;
}


void testobj_delete (struct testobj * testobj) {
    free(testobj);
}


struct testobj * testobj_copy (const struct testobj * testobj) {
    return testobj_create(testobj->value);
}


int testobj_cmp (const struct testobj * lhs, const struct testobj * rhs) {
    if (lhs->value < rhs->value)
        return -1;
    else if (lhs->value > rhs->value)
        return 1;
    return 0;
}