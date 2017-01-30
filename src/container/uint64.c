#include "uint64.h"

#include <stdlib.h>

const struct object_vtable uint64_vtable = {
    (void (*) (void *)) uint64_delete,
    (void * (*) (const void *)) uint64_copy,
    (int (*) (const void *, const void *)) uint64_cmp
};


struct uint64 * uint64_create (uint64_t value) {
    struct uint64 * uint64 = malloc(sizeof(struct uint64));
    object_init(&(uint64->oh), &uint64_vtable);
    uint64->value = value;
    return uint64;
}


void uint64_delete (struct uint64 * uint64) {
    free(uint64);
}


struct uint64 * uint64_copy (const struct uint64 * uint64) {
    return uint64_create(uint64->value);
}


int uint64_cmp (const struct uint64 * lhs, const struct uint64 * rhs) {
    if (lhs->value < rhs->value)
        return -1;
    else if (lhs->value > rhs->value)
        return 1;
    return 0;
}
