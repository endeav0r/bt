#include "testobj.h"

#include "container/varstore.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main () {
    struct varstore * varstore = varstore_create();

    assert(varstore_insert(varstore, "test1", 1) == 0);
    assert(varstore_insert(varstore, "test8", 8) == 4);
    assert(varstore_insert(varstore, "test16", 16) == 8);
    assert(varstore_insert(varstore, "test32", 32) == 12);
    assert(varstore_insert(varstore, "test64", 64) == 16);
    assert(varstore_insert(varstore, "test64_2", 64) == 24);

    assert(varstore_offset_create(varstore, "test64", 64) == 16);

    size_t offset;
    assert(varstore_offset(varstore, "test16", 16, &offset) == 0);
    assert(offset == 8);

    ODEL(varstore);

    return 0;
}