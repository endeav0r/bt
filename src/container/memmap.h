#ifndef memmap_HEADER
#define memmap_HEADER

#include "container/tree.h"
#include "object.h"

#include <stdint.h>
#include <stdlib.h>

#define memmap_R 1
#define memmap_W 2
#define memmap_X 4

struct memmap_page {
    const struct object * object;
    uint64_t address;
    uint8_t * data;
    size_t size;
    unsigned int permissions;
};


struct memmap_page * memmap_page_create (uint64_t address,
                                         size_t size,
                                         unsigned int permissions);
void                 memmap_page_delete (struct memmap_page * memmap_page);
struct memmap_page * memmap_page_copy   (const struct memmap_page * memmap_page);
int                  memmap_page_cmp    (const struct memmap_page * lhs,
                                         const struct memmap_page * rhs);


struct memmap {
    const struct object * object;
    struct tree * tree;
    unsigned int page_size;
};


struct memmap * memmap_create (unsigned int page_size);
void            memmap_delete (struct memmap * memmap);
struct memmap * memmap_copy   (const struct memmap * memmap);

/**
* Inserts the buf into the memmap at the given address with given permissions. If
* the pages do not exist they will be created. If buf_size is less than size,
* the pages will be created but will not be initialized.
* @param memmap the memmap struct
* @param address the address in memmap where we should begin creating memory
* @param size the size of memory to create in the memmap
* @param buf the memory to copy over. This can be NULL if buf_size == 0
* @param buf_size the size of buf. This must be less than size.
* @param permissions the mask of permissions to set this memory.
* @return 0 on success, non-zero on failure
*/
int memmap_map (struct memmap * memmap,
              uint64_t address,
              size_t size,
              const uint8_t * buf,
              size_t buf_size,
              unsigned int permissions);


int memmap_get_u8     (const struct memmap * memmap,
                       uint64_t address,
                       uint8_t * value);
int memmap_get_u16_le (const struct memmap * memmap,
                       uint64_t address,
                       uint16_t * value);
int memmap_get_u16_be (const struct memmap * memmap,
                       uint64_t address,
                       uint16_t * value);
int memmap_get_u32_le (const struct memmap * memmap,
                       uint64_t address,
                       uint32_t * value);
int memmap_get_u32_be (const struct memmap * memmap,
                       uint64_t address,
                       uint32_t * value);
int memmap_get_u64_le (const struct memmap * memmap,
                       uint64_t address,
                       uint64_t * value);
int memmap_get_u64_be (const struct memmap * memmap,
                       uint64_t address,
                       uint64_t * value);


int memmap_set_u8     (struct memmap * memmap,
                       uint64_t address,
                       uint8_t value);
int memmap_set_u16_le (struct memmap * memmap,
                       uint64_t address,
                       uint16_t value);
int memmap_set_u16_be (struct memmap * memmap,
                       uint64_t address,
                       uint16_t value);
int memmap_set_u32_le (struct memmap * memmap,
                       uint64_t address,
                       uint32_t value);
int memmap_set_u32_be (struct memmap * memmap,
                       uint64_t address,
                       uint32_t value);
int memmap_set_u64_le (struct memmap * memmap,
                       uint64_t address,
                       uint64_t value);
int memmap_set_u64_be (struct memmap * memmap,
                       uint64_t address,
                       uint64_t value);


#endif
