#include "testobj.h"

#include "container/tree.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main () {
    struct tree * tree = tree_create();

    unsigned int i;
    for (i = 0; i < 16; i++) {
        tree_insert_(tree, testobj_create(i));
    }

    for (i = 0; i < 16; i++) {
        struct testobj * testobj = testobj_create(i);
        struct testobj * fetched;
        fetched = (struct testobj *) tree_fetch(tree, testobj);
        assert(fetched != NULL);
        assert(fetched->value == i);
        ODEL(testobj);
    }

    struct tree * copy = OCOPY(tree);

    for (i = 0; i < 16; i++) {
        struct testobj * testobj = testobj_create(i);
        struct testobj * fetched;
        fetched = (struct testobj *) tree_fetch(copy, testobj);
        assert(fetched != NULL);
        assert(fetched->value == i);
        ODEL(testobj);
    }

    ODEL(copy);

    for (i = 0; i < 16; i++) {
        if (i % 2 == 1)
            continue;
        struct testobj * testobj = testobj_create(i);
        assert(tree_remove(tree, testobj) == 0);
        ODEL(testobj);
    }

    for (i = 0; i < 16; i++) {
        struct testobj * testobj = testobj_create(i);
        struct testobj * fetched;
        fetched = (struct testobj *) tree_fetch(tree, testobj);
        if (i % 2 == 0)
            assert(fetched == NULL);
        else {
            assert(fetched != NULL);
            assert(fetched->value == i);
        }
        ODEL(testobj);
    }

    ODEL(tree);

    return 0;
}