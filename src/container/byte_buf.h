#ifndef byte_buf_HEADER
#define byte_buf_HEADER

#include "object.h"

#include <stdint.h>

struct buf {
    const struct object * object;
    uint8_t * buf;
    size_t length; // number of bytes used
    size_t allocated_size; // number of bytes allocated
};


struct byte_buf * byte_buf_create ();
void              byte_buf_delete (struct byte_buf * byte_buf);
struct byte_buf * byte_buf_copy   (const struct byte_buf * byte_buf);

int byte_buf_append       (struct byte_buf * byte_buf, uint8_t byte);
int byte_buf_append_le16  (struct byte_buf * byte_buf, uint16_t uint16);
int byte_buf_append_le32  (struct byte_buf * byte_buf, uint32_t uint32);
int byte_buf_append_le64  (struct byte_buf * byte_buf, uint32_t uint64);
int byte_buf_append_bytes (struct byte_buf * byte_buf,
                           const uint8_t * bytes,
                           size_t bytes_size);
int byte_buf_append_byte_buf (struct byte_buf * byte_buf,
                              const struct byte_buf * src);

size_t          byte_buf_length (const struct byte_buf * byte_buf);
const uint8_t * byte_buf_bytes  (const struct byte_buf * byte_buf);

#endif