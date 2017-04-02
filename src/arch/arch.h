#ifndef arch_HEADER
#define arch_HEADER

#include "container/list.h"
#include "container/varstore.h"

#include <stdlib.h>

struct arch_source {
    const char  * (* ip_variable_identifier) ();
    unsigned int  (* ip_variable_bits) ();
    struct list * (* translate_ins)   (
        const void * buf,
        size_t size,
        uint64_t address
    );
    struct list * (* translate_block) (
        const void * buf,
        size_t size,
        uint64_t address
    );
};

struct arch_target {
    struct byte_buf * (* assemble) (struct list * btins_list,
                                  struct varstore * varstore);
    unsigned int (* execute) (const void * code, struct varstore * varstore);
};


#endif