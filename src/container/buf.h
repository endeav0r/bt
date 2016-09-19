#ifndef buf_HEADER
#define buf_HEADER

#include "object.h"

#include <stdint.h>

struct buf {
    const struct object * object;
    uint8_t * buf;
    size_t length;
};


struct buf * buf_create (size_t length);
void         buf_delete (struct buf * buf);
struct buf * buf_copy   (const struct buf * buf);

struct buf * buf_slice (const struct buf * buf, size_t offset, size_t size);
const void * buf_get   (const struct buf * buf, size_t offset, size_t size);
int buf_set (const struct buf * buf,
             size_t offset,
             size_t length,
             const void * data);



#endif