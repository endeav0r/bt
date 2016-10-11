#include "testobj.h"

#include "container/list.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main () {
    struct list * list = list_create();

    struct testobj * testobj = testobj_create(1);
    list_append(list, testobj);
    list_append_(list, testobj);

    struct list * copy = OCOPY(list);

    list_append_(list, testobj_create(2));
    list_prepend_(list, testobj_create(0));

    testobj = (struct testobj *) list_front(list);
    assert(testobj->value == 0);
    testobj = (struct testobj *) list_back(list);
    assert(testobj->value == 2);

    unsigned int values [] = {0, 1, 1, 2};

    unsigned int i = 0;
    struct list_it * it;
    for (it = list_it(list); it != NULL; it = list_it_next(it)) {
        testobj = (struct testobj *) list_it_obj(it);
        assert(testobj->value == values[i++]);
    }

    list_pop_front(list);
    testobj = (struct testobj *) list_front(list);
    assert(testobj->value == 1);

    list_append_(list, testobj_create(3));
    list_pop_back(list);
    testobj = (struct testobj *) list_back(list);
    assert(testobj->value == 2);

    ODEL(copy);
    ODEL(list);

    return 0;
}