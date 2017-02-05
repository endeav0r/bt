#ifndef jit_HEADER
#define jit_HEADER

#include <stdint.h>
#include <stdlib.h>

#include "arch/arch.h"
#include "container/memmap.h"
#include "container/tree.h"
#include "container/varstore.h"
#include "object.h"
#include "platform/platform.h"

#define INITIAL_MMAP_SIZE (1024 * 1024 * 32)
#define INITIAL_VAR_MEM_SIZE (8 * 128)

struct jit_block {
    struct object_header oh;
    uint64_t vaddr;
    size_t mm_offset;
    size_t size;
};


struct jit {
    struct object_header oh;
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
    const struct platform * platform;
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
                         const struct arch_target * arch_target,
                         const struct platform * platform);
void         jit_delete (struct jit * jit);
struct jit * jit_copy   (const struct jit * jit);

int jit_set_code (struct jit * jit,
                  uint64_t vaddr,
                  const void * code,
                  size_t code_size);

const void * jit_get_code (struct jit * jit, uint64_t vaddr);

/*
* Executes the code based upon varstore and memmap until the program
* successfully terminates or an error condition is reached.
* @param jit A pointer to the jit we will execute this program in.
* @param varstore A pointer to the varstore we are jitting over.
* @param memmap A pointer to the memmap we are jitting over.
* @return -1 if we could not fetch the instruction pointer from the varstore,
          -2 if the instruction pointer was an invalid bit width,
          -3 if we failed to translate instructions from memmap to bins
          -4 if we failed to assemble the bins to the target asm
          -5 if there was a platform error
          1 if there was an error reading from the MMU
          2 if there was an error writing to the MMU
          0 if execution stopped normally.
*/
int jit_execute (struct jit * jit,
                 struct varstore * varstore,
                 struct memmap * memmap);

#endif
