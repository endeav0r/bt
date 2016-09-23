#ifndef jit_HEADER
#define jit_HEADER

#include <stdint.h>

#include "container/tree.h"

#define INITIAL_MMAP_SIZE (1024 * 1024 * 32)
#define INITIAL_VAR_MEM_SIZE (8 * 128)

struct jit_block {
    const struct object * object;
    uint64_t vaddr;
    size_t mm_offset;
    size_t size;
};


struct jit_var {
    const struct object * object;
    const char * identifier;
    size_t offset;
    size_t size;
}


struct jit {
    const struct object * object;
    /* A tree of jit_block structs we use to find jit code for blocks by
       virtual address. */
    struct tree * blocks;
    /* r/w/x memory used to store jit code */
    uint8_t * mmap_mem;
    /* size of mmap_mem */
    size_t mmap_size;
    /* A tree of jit_var structs we use to track variables and their offset into
       the var_mem buffer */
    struct tree * variables;
    /* r/w memory used to store jit variables */
    uint8_t * var_mem;
    /* The size of allocated bytes for var_mem */
    size_t var_mem_size;
    /* The number of bytes used by var_mem. I.E. this is the index to the next
       byte in the var_mem buffer we can use to allocate a new variable */
    size_t var_mem_top;
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


struct jit * jit_create ();
void         jit_delete (struct jit * jit);
struct jit * jit_copy   (const struct jit * jit);

#endif