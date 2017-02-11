#ifndef mips_source_HEADER
#define mips_source_HEADER

#include "arch/arch.h"
#include "container/list.h"

#include <stdlib.h>

/*
* Halt Codes
* 1 = Integer Overflow trap
*/

extern const struct arch_source arch_source_mips;

const char *  mips_ip_variable_identifier ();
unsigned int  mips_ip_variable_bits ();
struct list * mips_translate_ins   (const void * buf, size_t size);
struct list * mips_translate_block (const void * buf, size_t size);

#endif
