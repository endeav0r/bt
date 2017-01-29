#ifndef buf_HEADER
#define buf_HEADER

/**
* buf creates a basic, generic buffer object for binary translator. If ownership
* of a buffer is going to be passed, it is best to pass it in a buf object.
*/

#include "object.h"

#include <stdint.h>
#include <stdlib.h>

struct buf {
    struct object_header oh;
    uint8_t * buf;
    size_t length;
};


/**
* Create a buf object with a buffer of specified length.
* @param length The length of the buffer.
* @return An initialized buffer object with a buffer of size length.
*/
struct buf * buf_create (size_t length);

/**
* Delete a buffer object. You should not call this, call ODEL()
* @param buf The buf to delete.
*/
void buf_delete (struct buf * buf);

/**
* Copy a buffer object. You should not call this, call OCOPY()
* @param buf A pointer to the buf to copy.
* @return A copy of the given buf.
*/
struct buf * buf_copy (const struct buf * buf);

/**
* Get the length of the buf in bytes.
* @param buf The buf we want the length of.
* @return The length of the buf in bytes.
*/
size_t buf_length (const struct buf * buf);

/**
* Slices out a sub-portion of a given buf, and returns it in a new buf object.
* @param buf The buf we are slicing out of.
* @param offset An offset into the buf, in bytes, where our slice will begin.
* @param size The size of our slice in bytes.
* @return A pointer to a buf which contains the slice, or NULL if the slice
*         would not fall within the bounds of this buf.
*/
struct buf * buf_slice (const struct buf * buf, size_t offset, size_t size);

/**
* Returns a pointer to the data contained within a buf.
* @param buf A pointer to the buf we want data out of.
* @param offset An offset into the buf, in bytes, to the data we want.
* @param size The number of bytes we want to retrieve from the buf.
* @return A pointer to the data, or NULL if the requested offset and size
*         references data outside the bounds of the buf.
*/
const void * buf_get   (const struct buf * buf, size_t offset, size_t size);

/**
* Sets data in the buf.
* @param buf The buf we are setting data in.
* @param offset An offset into the buf, in bytes, where this operation begins.
* @param length The length of the data we are setting, in bytes.
* @param data A pointer to the bytes we want to set in this buf.
* @return 0 on success, or non-zero on failure. This call will fail if the
*         offset and length will cause data to be written outside the bounds of
*         this buf.
*/
int buf_set (const struct buf * buf,
             size_t offset,
             size_t length,
             const void * data);



#endif
