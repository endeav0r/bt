#ifndef jit_HEADER
#define jit_HEADER

#include <stdint.h>
#include <stdlib.h>

#include "arch/arch.h"
#include "container/tree.h"
#include "object.h"

#define INITIAL_MMAP_SIZE (1024 * 1024 * 32)
#define INITIAL_VAR_MEM_SIZE (8 * 128)

struct jit_block {
    const struct object * object;
    uint64_t vaddr;
    size_t mm_offset;
    size_t size;
};


struct jit {
    const struct object * object;
    /* A tree of jit_block structs we use to find jit code for blocks by
       virtual address. */
    struct tree * blocks;
    /* r/w/x memory used to store jit code */
    uint8_t * mmap_mem;
    /* size of mmap_mem */
    size_t mmap_size;
    /* offset to next available space in mmap_mem */
    size_t mmap_next;

    const struct arch_source * arch_source;
    const struct arch_target * arch_target;
};


struct jit_block * jit_block_create (uint64_t vaddr,
                                     size_t mm_offset,
                                     size_t size);
void               jit_block_delete (struct jit_block * jit_block);
struct jit_block * jit_block_copy   (const struct jit_block * jit_block);
int                jit_block_cmp (const struct jit_block * lhs,
                                  const struct jit_block * rhs);


struct jit_var * jit_var_create (const char * identifier,
                                 size_t offset,
                                 size_t size);
void             jit_var_delete (struct jit_var * jit_var);
struct jit_var * jit_var_copy   (const struct jit_var * jit_var);
int              jit_var_cmp    (const struct jit_var * lhs,
                                 const struct jit_var * rhs);

struct jit * jit_create (const struct arch_source * arch_source,
                         const struct arch_target * arch_target);
void         jit_delete (struct jit * jit);
struct jit * jit_copy   (const struct jit * jit);

int jit_set_code (struct jit * jit,
                  uint64_t vaddr,
                  const void * code,
                  size_t code_size);

const void * jit_get_code (struct jit * jit, uint64_t vaddr);

#endif