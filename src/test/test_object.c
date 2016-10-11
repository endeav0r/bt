#include "testobj.h"

#include <assert.h>
#include <stdio.h>

int main () {
    struct testobj * lhs = testobj_create(1);
    struct testobj * rhs = testobj_create(2);

    struct testobj * copy = OCOPY(rhs);

    assert(lhs->value == 1);
    assert(rhs->value == 2);

    assert(OCMP(lhs, rhs) < 0);
    assert(OCMP(copy, lhs) > 0);
    assert(OCMP(copy, rhs) == 0);

    ODEL(lhs);
    ODEL(rhs);
    ODEL(copy);

    return 0;
}