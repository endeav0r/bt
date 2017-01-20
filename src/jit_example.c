#include "arch/target/amd64.h"
#include "bt/bins.h"
#include "container/byte_buf.h"
#include "container/list.h"
#include "container/varstore.h"

#include <unistd.h>

int main () {
    struct list * list = list_create();

    /*
    list_append_(list, bins_or(boper_variable(32, "eax"),
                               boper_constant(32, 0),
                               boper_constant(32, 7)));
    list_append_(list, bins_or(boper_variable(32, "ecx"),
                               boper_constant(32, 0),
                               boper_constant(32, 9)));
    list_append_(list, bins_add(boper_variable(32, "eax"),
                                boper_variable(32, "eax"),
                                boper_variable(32, "ecx")));
    */

    list_append(list, bins_add_(boper_variable(16, "rip"),
                                boper_variable(16, "rip"),
                                boper_constant(16, 4)));
    list_append(list, bins_or_(boper_variable(16, "r0"),
                               boper_constant(16, 8),
                               boper_constant(16, 0)));
    list_append(list, bins_add_(boper_variable(16, "rip"),
                                boper_variable(16, "rip"),
                                boper_constant(16, 4)));
    list_append(list, bins_sub_(boper_variable(16, "rsp"),
                                boper_variable(16, "rsp"),
                                boper_constant(16, 2)));
    list_append(list, bins_shr_(boper_variable(16, "t16"),
                                boper_variable(16, "r0"),
                                boper_constant(16, 8)));

    struct varstore * varstore = varstore_create();

    struct byte_buf * bb = amd64_assemble(list, varstore);

    int written = write(1, byte_buf_bytes(bb), byte_buf_length(bb));
    if (written != byte_buf_length(bb))
      return -1;

    ODEL(varstore);
    ODEL(list);
    ODEL(bb);

    return 0;
}
