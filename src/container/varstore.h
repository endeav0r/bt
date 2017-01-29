#ifndef varstore_HEADER
#define varstore_HEADER

#include "container/tree.h"
#include "object.h"

#include <stdint.h>
#include <stdlib.h>

struct varstore_node {
    struct object_header oh;
    char * identifier;
    size_t bits;
    size_t offset;
};


struct varstore_node * varstore_node_create (const char * identifier,
                                             size_t bits,
                                             size_t offset);
void                   varstore_node_delete (struct varstore_node * vn);
struct varstore_node * varstore_node_copy   (const struct varstore_node * vn);
int varstore_node_cmp (const struct varstore_node * lhs,
                       const struct varstore_node * rhs);


struct varstore {
    struct object_header oh;
    struct tree * tree;
    uint8_t * data_buf;
    size_t next_offset;
    size_t data_buf_size;
};


struct varstore * varstore_create ();
void              varstore_delete (struct varstore * varstore);
struct varstore * varstore_copy   (const struct varstore * varstore);

size_t varstore_insert (struct varstore * varstore,
                        const char * identifier,
                        size_t bits);
int varstore_offset (const struct varstore * varstore,
                     const char * identifier,
                     size_t bits,
                     size_t * offset);

/**
* Retrieves a variable from the varstore, up to 64-bits in size, and places
* it's value in the uint64_t pointed to by value.
* @param varstore The varstore to retrieve this variable from.
* @param identifier The string identifier of the variable.
* @param bits The size of the variable in bits.
* @param value A uint64_t to place the value in.
* @return Returns 0 on success, or non-zero on failure, such as the variable is
*         not found, or an invalid bits size was passed.
*/
int varstore_value (const struct varstore * varstore,
                    const char * identifier,
                    size_t bits,
                    uint64_t * value);

void * varstore_data_buf (struct varstore * varstore);

// will create the variable if it doesn't exist, always returns an offset
size_t varstore_offset_create (struct varstore * varstore,
                               const char * identifier,
                               size_t bits);

#endif
