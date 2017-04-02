#ifndef hsvm_source_HEADER
#define hsvm_source_HEADER

#include "arch/arch.h"
#include "container/list.h"

#include <stdlib.h>

extern const struct arch_source arch_source_hsvm;

const char *  hsvm_ip_variable_identifier ();
unsigned int  hsvm_ip_variable_bits ();
struct list * hsvm_translate_ins   (
    const void * buf,
    size_t size,
    uint64_t address
);
struct list * hsvm_translate_block (
    const void * buf,
    size_t size,
    uint64_t address
);

#endif