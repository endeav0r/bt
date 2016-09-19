#ifndef arch_hsvm_HEADER
#define arch_hsvm_HEADER

#include "container/list.h"

extern const struct arch arch_hsvm;

struct list * arch_hsvm_translate_ins   (const void * buf, size_t size);
struct list * arch_hsvm_translate_block (const void * buf, size_t size);

#endif