#ifndef asarm_HEADER
#define asarm_HEADER

#include "arch/arch.h"
#include "container/list.h"

#include <stdlib.h>
#include <capstone/capstone.h>

extern const struct arch_source arch_source_arm;

const char *  arm_ip_variable_identifier ();

unsigned int  arm_ip_variable_bits ();

struct list * arm_translate_ins   (
    const void * buf,
    size_t size,
    uint64_t address
);

struct list * arm_translate_block (
    const void * buf,
    size_t size,
    uint64_t address
);



struct boper * asarm_cs_reg (unsigned int cs_reg);

struct list *  asarm_ins_cond (const cs_arm * arm, unsigned int ins_n);
struct boper * asarm_operand  (struct list * list, cs_arm_op * op);

#endif