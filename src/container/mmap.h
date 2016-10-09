#ifndef mmap_HEADER
#define mmap_HEADER

#include "container/tree.h"
#include "object.h"

#include <stdint.h>

#define mmap_R 1
#define mmap_W 2
#define mmap_X 4

struct mmap_page {
    const struct object * object;
    uint64_t address;
    uint8_t * data;
    size_t size;
    unsigned int permissions;
};


struct mmap_page * mmap_page_create (uint64_t address,
                                     size_t size,
                                     unsigned int permissions);
void               mmap_page_delete (struct mmap_page * mmap_page);
struct mmap_page * mmap_page_copy   (const struct mmap_page * mmap_page);
int                mmap_page_cmp    (const struct mmap_page * lhs,
                                     const struct mmap_page * rhs);


struct mmap {
    const struct object * object;
    struct tree * tree;
    unsigned int page_size;
};


struct mmap * mmap_create  (unsigned int page_size);
void          mmap_destroy (struct mmap * mmap);
struct mmap * mmap_copy    (const struct mmap * mmap);

/**
* Inserts the buf into the mmap at the given address with given permissions. If
* the pages do not exist they will be created. If buf_size is less than size,
* the pages will be created but will not be initialized.
* @param mmap the mmap struct
* @param address the address in mmap where we should begin creating memory
* @param size the size of memory to create in the mmap
* @param buf the memory to copy over. This can be NULL if buf_size == 0
* @param buf_size the size of buf. This must be less than size.
* @param permissions the mask of permissions to set this memory.
* @return 0 on success, non-zero on failure
*/
int mmap_map (struct mmap * mmap,
              uint64_t address,
              size_t size,
              const uint8_t * buf,
              size_t buf_size,
              unsigned int permissions);


int mmap_get_u8     (const struct mmap * mmap,
                     uint64_t address,
                     uint8_t * value);
int mmap_get_u16_le (const struct mmap * mmap,
                     uint64_t address,
                     uint16_t * value);
int mmap_get_u16_be (const struct mmap * mmap,
                     uint64_t address,
                     uint16_t * value);
int mmap_get_u32_le (const struct mmap * mmap,
                     uint64_t address,
                     uint32_t * value);
int mmap_get_u32_be (const struct mmap * mmap,
                     uint64_t address,
                     uint32_t * value);
int mmap_get_u64_le (const struct mmap * mmap,
                     uint64_t address,
                     uint64_t * value);
int mmap_get_u64_be (const struct mmap * mmap,
                     uint64_t address,
                     uint64_t * value);


int mmap_set_u8     (const struct mmap * mmap,
                     uint64_t address,
                     uint8_t value);
int mmap_set_u16_le (const struct mmap * mmap,
                     uint64_t address,
                     uint16_t * value);
int mmap_set_u16_be (const struct mmap * mmap,
                     uint64_t address,
                     uint16_t * value);
int mmap_set_u32_le (const struct mmap * mmap,
                     uint64_t address,
                     uint32_t * value);
int mmap_set_u32_be (const struct mmap * mmap,
                     uint64_t address,
                     uint32_t * value);
int mmap_set_u64_le (const struct mmap * mmap,
                     uint64_t address,
                     uint64_t * value);
int mmap_set_u64_be (const struct mmap * mmap,
                     uint64_t address,
                     uint64_t * value);


#endif
