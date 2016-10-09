#ifndef target_HEADER
#define target_HEADER

#include "container/mmap.h"
#include "container/tree.h"

#include <stdint.h>
#include <stdlib.h>

struct target_variable {
    const struct object * object;
    // text identifier for variable ("rax" or "tmp")
    const char * identifier;
    // offset into variable store where we can find this variable
    size_t offset;
    // size of this variable in bits
    unsigned int bits;
};

struct target_codeblock {
    const struct object * object;
    // this is the virtual address where this code would live in real life
    uint64_t virtual_address;
    // this is the size of the block of real-world code
    size_t code_size;
    // this is the offset into rwx code where our instrumented code lives
    size_t rwx_offset;
    // this is the size of the rwx code in bytes
    size_t rwx_code_size;
};

struct target {
    const struct object * object;
    // this is the block of memory where we store variable
    uint8_t * variable_space;
    // size of the variable store in bytes
    size_t variable_space_size;
    // tree that stores our target variables so we can find them in variable
    // space
    struct tree * variables;
    // tree that stores our codeblocks
    struct tree * codeblocks;
    // mmaped rwx memory, or the memory where instrumented code goes
    uint8_t * rwx_mem;
    // size of mmaped rwx memory
    size_t rwx_mem_size;
    // the mmap of the initial memory state
    struct mmap * mmap;
};


struct target_variable * target_variable_create (
    const char * identifier,
    size_t offset,
    unsigned int bits
);
void target_variable_delete (struct target_variable * target_variable);
struct target_variable * target_variable_copy (
    const struct target_variable * target_variable
);
int target_variable_cmp (
    const struct target_variable * lhs,
    const struct target_variable * rhs
);


struct target_codeblock * target_codeblock_create (
    uint64_t virtual_address,
    size_t code_size,
    size_t rwx_offset,
    size_t rwx_code_size
);
void target_codeblock_delete (struct target_codeblock * target_codeblock);
struct target_codeblock * target_codeblock_copy (
    const struct target_codeblock * target_codeblock
);
int target_codeblock_cmp (
    const struct target_codeblock * lhs,
    const struct target_codeblock * rhs
);


struct target * target_create (struct mmap * mmap_);
void target_delete (struct target * target);
struct target * target_copy (const struct target * target);

/*
* Fetches the variable which points to the instruction pointer as given by the
* source_arch, checks to see if this address is instrumented. If it is, executes
* the instrumented code. If the address is not instrumented, translates from
* internal mmap to btins, assembles btins to target architecture, sets this
* codeblock, and then executes the codeblock.
* @param target the target object
* @param source_arch struct which points to the source architecture.
* @param target_arch struct which points to the target architecture (should be
*                    the same architecture of the host machine running bt).
* @return 0 on success, non-zero on failure
*/
int target_execute (struct target * target,
                    const struct arch * source_arch,
                    const struct arch * target_arch);

int target_read_byte (struct target * target,
                      uint64_t address,
                      uint8_t * byte);

int target_write_byte (struct target * target,
                       uint64_t address,
                       uint8_t * byte);

#endif