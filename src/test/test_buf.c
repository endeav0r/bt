#include "testobj.h"

#include "container/buf.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

const char * test_string = "this is a test";

int main () {
    size_t length = strlen(test_string);

    struct buf * buf = buf_create (32);

    buf_set(buf, 0, length + 1, test_string);

    struct buf * copy = OCOPY(buf);

    assert(strcmp((const char *) buf_get(copy, 0, length + 1), test_string) == 0);

    struct buf * slice = buf_slice(copy, 5, 10);

    assert(strcmp((const char *) buf_get(slice, 0, 10), "is a test") == 0);

    buf_set(slice, 2, 2, "nt");

    assert(strcmp((const char *) buf_get(slice, 0, 10), "isnt test") == 0);

    ODEL(slice);
    ODEL(copy);
    ODEL(buf);

    return 0;
}