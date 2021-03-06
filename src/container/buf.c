#include "buf.h"

#include <stdlib.h>
#include <string.h>

const struct object_vtable buf_vtable = {
    (void (*) (void *)) buf_delete,
    (void * (*) (const void *)) buf_copy,
    NULL
};


struct buf * buf_create (size_t length) {
    struct buf * buf = malloc(sizeof(struct buf));
    object_init(&(buf->oh), &buf_vtable);
    buf->length = length;
    buf->buf = malloc(length);

    return buf;
}


void buf_delete (struct buf * buf) {
    free(buf->buf);
    free(buf);
}


struct buf * buf_copy (const struct buf * buf) {
    struct buf * copy = buf_create(buf->length);
    memcpy(copy->buf, buf->buf, buf->length);
    return copy;
}


size_t buf_length (const struct buf * buf) {
    return buf->length;
}


struct buf * buf_slice (const struct buf * buf, size_t offset, size_t size) {
    if (offset + size >= buf->length)
        return NULL;

    struct buf * slice = buf_create(size);
    memcpy(slice->buf, &(buf->buf[offset]), size);

    return slice;
}


const void * buf_get (const struct buf * buf, size_t offset, size_t size) {
    if (offset + size > buf->length) {
        return NULL;
    }
    return &(buf->buf[offset]);
}


int buf_set (const struct buf * buf,
             size_t offset,
             size_t length,
             const void * data) {
    if (offset + length > buf->length)
        return -1;
    memcpy(&(buf->buf[offset]), data, length);
    return 0;
}
