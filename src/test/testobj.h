#ifndef testobj_HEADER
#define testobj_HEADER

#include "object.h"

struct testobj {
    struct object_header oh;
    unsigned int value;
};

struct testobj * testobj_create (unsigned int value);
void             testobj_delete (struct testobj * testobj);
struct testobj * testobj_copy   (const struct testobj * testobj);
int              testobj_cmp    (const struct testobj * lhs,
                                 const struct testobj * rhs);

#endif
