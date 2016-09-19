#ifndef loader_HEADER
#define loader_HEADER

#include "object.h"

struct loader_object {
    struct object_header object;
    uint64_t (* entry) (const struct loader *);
    struct mmap * (* mmap) (const struct loader *);
};

#endif