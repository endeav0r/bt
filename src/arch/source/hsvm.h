#ifndef hsvm_source_HEADER
#define hsvm_source_HEADER

#include "container/list.h"

#include <stdlib.h>

struct list * hsvm_translate_ins   (const void * buf, size_t size);
struct list * hsvm_translate_block (const void * buf, size_t size);

#endif