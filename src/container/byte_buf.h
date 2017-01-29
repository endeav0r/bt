#ifndef byte_buf_HEADER
#define byte_buf_HEADER

/**
* byte_buf creates a buildable buffer, useful for creating buffers one, or
* multiple, bytes at a time.
*
* byte_buf is the go-to structure for things like building sequences of jit
* instructions.
*/

#include "object.h"

#include <stdint.h>
#include <stdlib.h>

struct byte_buf {
    struct object_header oh;
    uint8_t * buf;
    size_t length; // number of bytes used
    size_t allocated_size; // number of bytes allocated
};


/**
* Creates a byte_buf.
* @return A new, empty byte_buf.
*/
struct byte_buf * byte_buf_create ();

/**
* Deletes a byte_buf. Don't call this, call ODEL().
* @param byte_buf The byte_buf to delete.
*/
void byte_buf_delete (struct byte_buf * byte_buf);

/**
* Copies a byte_buf. Don't call this, call OCOPY().
* @param byte_buf A pointer to the byte_buf to copy.
* return A copy of the passed byte_buf.
*/
struct byte_buf * byte_buf_copy (const struct byte_buf * byte_buf);

/**
* Append a byte to the byte_buf.
* @param byte_buf The byte buf to which we are appending a byte.
* @param byte The byte we are appending to the byte_buf.
* @return 0 on success, or non-zero on error.
*/
int byte_buf_append (struct byte_buf * byte_buf, uint8_t byte);

/**
* Appends a uint16_t value to the byte_buf in little-endian order.
* @param byte_buf The byte buf to which we are appending a uint16_t.
* @param uint16 The value we are appending.
* @return 0 on success, or non-zero on error.
*/
int byte_buf_append_le16 (struct byte_buf * byte_buf, uint16_t uint16);

/**
* Appends a uint32_t value to the byte_buf in little-endian order.
* @param byte_buf The byte buf to which we are appending a uint32_t.
* @param uint32 The value we are appending.
* @return 0 on success, or non-zero on error.
*/
int byte_buf_append_le32 (struct byte_buf * byte_buf, uint32_t uint32);

/**
* Appends a uint64_t value to the byte_buf in little-endian order.
* @param byte_buf The byte buf to which we are appending a uint64_t.
* @param uint64 The value we are appending.
* @return 0 on success, or non-zero on error.
*/
int byte_buf_append_le64 (struct byte_buf * byte_buf, uint64_t uint64);

/**
* Appends a raw bytes to the end of the byte_buf.
* @param byte_buf The byte buf to which we are appending.
* @param bytes A pointer to the bytes we are appending.
* @param bytes_size The size of bytes in bytes.
* @return 0 on success, non-zero on failure.
*/
int byte_buf_append_bytes (struct byte_buf * byte_buf,
                           const uint8_t * bytes,
                           size_t bytes_size);

/**
* Appends the contents of another byte buf to this byte buf.
* @param byte_buf The destination buf we are appending to.
* @param src The byte_buf which will be appended to byte_buf.
* @return 0 on success, non-zero on failure.
*/
int byte_buf_append_byte_buf (struct byte_buf * byte_buf,
                              const struct byte_buf * src);

/**
* Gets the length of the contents of a byte_buf.
* @param byte_buf The byte_buf we want the length of.
* @return The length of the contents of the byte_buf in bytes.
*/
size_t byte_buf_length (const struct byte_buf * byte_buf);

/**
* Gets the contents of a byte_buf,
* @param byte_buf The byte_buf we want the contents of.
* @return The contents of the byte_buf.
*/
const uint8_t * byte_buf_bytes (const struct byte_buf * byte_buf);

#endif
