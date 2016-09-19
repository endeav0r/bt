#ifndef arch_HEADER
#define arch_HEADER

#include "container/list.h"

struct arch {
    struct list * (* translate_ins)   (const void * buf, size_t size);
    struct list * (* translate_block) (const void * buf, size_t size);
};


#endif