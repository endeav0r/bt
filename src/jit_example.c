#include "arch/target/amd64.h"
#include "bt/bins.h"
#include "container/byte_buf.h"
#include "container/list.h"
#include "container/varstore.h"

#include <unistd.h>

int main () {
    struct list * list = list_create();

    list_append_(list, bins_or(boper_variable(32, "eax"),
                               boper_constant(32, 0),
                               boper_constant(32, 7)));
    list_append_(list, bins_or(boper_variable(32, "ecx"),
                               boper_constant(32, 0),
                               boper_constant(32, 9)));
    list_append_(list, bins_add(boper_variable(32, "eax"),
                                boper_variable(32, "eax"),
                                boper_variable(32, "ecx")));

    struct varstore * varstore = varstore_create();

    struct byte_buf * bb = amd64_assemble(list, varstore);

    write(1, byte_buf_bytes(bb), byte_buf_length(bb));

    ODEL(varstore);
    ODEL(list);
    ODEL(bb);

    return 0;
}