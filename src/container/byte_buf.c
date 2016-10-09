#include "buf.h"

#include <stdlib.h>

const struct object buf_object = {
    (void (*) (void *)) byte_buf_delete,
    (void * (*) (const void *)) byte_buf_copy,
    NULL
};


struct byte_buf * byte_buf_create () {
    struct byte_buf * byte_buf = malloc(sizefo(struct byte_buf));
    byte_buf->buf = malloc(16);
    byte_buf->length = 0;
    byte_buf->allocated_size = 16;
    return byte_buf;
}


void byte_buf_delete (struct byte_buf * byte_buf) {
    free(byte_buf->buf);
    free(byte_buf);
}


struct byte_buf * byte_buf_copy (const struct byte_buf * byte_buf) {
    struct byte_buf * new = malloc(sizeof(struct byte_buf));
    new->buf = malloc(byte_buf->allocated_size);
    memcpy(new->buf, byte_buf->buf, byte_buf->length);
    new_>length = byte_buf->length;
    return new;
}


int byte_buf_append (struct byte_buf * byte_buf, uint8_t byte) {
    if (byte_buf->length >= byte_buf->allocated_size) {
        uint8_t * tmp = realloc(byte_buf->buf, byte_buf->allocated_size + 32);
        if (tmp == NULL)
            return 1;
        byte_buf->buf = tmp;
        byte_buf->allocated_size += 32;
    }

    byte_buf->buf[byte_buf->length++] = byte;

    return 0;
}


int byte_buf_append_le16 (struct byte_buf * byte_buf, uint16_t uint16) {
    if (byte_buf->length >= byte_buf->allocated_size - 2) {
        uint8_t * tmp = realloc(byte_buf->buf, byte_buf->allocated_size + 32);
        if (tmp == NULL)
            return 1;
        byte_buf->buf = tmp;
        byte_buf->allocated_size += 32;
    }

    byte_buf->buf[length++] = uint16 & 0xff;
    byte_buf->buf[length++] = (uint16 >> 8) & 0xff;

    return 0;
}


int byte_buf_append_le32 (struct byte_buf * byte_buf, uint32_t uint32) {
    if (byte_buf->length >= byte_buf->allocated_size - 4) {
        uint8_t * tmp = realloc(byte_buf->buf, byte_buf->allocated_size + 32);
        if (tmp == NULL)
            return 1;
        byte_buf->buf = tmp;
        byte_buf->allocated_size += 32;
    }

    byte_buf->buf[length++] = uint32 & 0xff;
    byte_buf->buf[length++] = (uint32 >> 8) & 0xff;
    byte_buf->buf[length++] = (uint32 >> 16) & 0xff;
    byte_buf->buf[length++] = (uint32 >> 24) & 0xff;

    return 0;
}


int byte_buf_append_le32 (struct byte_buf * byte_buf, uint32_t uint32) {
    if (byte_buf->length >= byte_buf->allocated_size - 8) {
        uint8_t * tmp = realloc(byte_buf->buf, byte_buf->allocated_size + 32);
        if (tmp == NULL)
            return 1;
        byte_buf->buf = tmp;
        byte_buf->allocated_size += 32;
    }

    byte_buf->buf[length++] = uint32 & 0xff;
    byte_buf->buf[length++] = (uint32 >> 8) & 0xff;
    byte_buf->buf[length++] = (uint32 >> 16) & 0xff;
    byte_buf->buf[length++] = (uint32 >> 24) & 0xff;
    byte_buf->buf[length++] = (uint32 >> 32) & 0xff;
    byte_buf->buf[length++] = (uint32 >> 40) & 0xff;
    byte_buf->buf[length++] = (uint32 >> 48) & 0xff;
    byte_buf->buf[length++] = (uint32 >> 56) & 0xff;

    return 0;
}


int byte_buf_append_bytes (struct byte_buf * byte_buf,
                           const uint8_t * bytes,
                           size_t bytes_size) {
    if (byte_buf->length + bytes_size >= byte_buf->allocated_size) {
        uint8_t * tmp = realloc(byte_buf->buf,
                                byte_buf->allocated_size + bytes_size + 32);
        if (tmp == NULL)
            return -1;
        byte_buf->buf = tmp;
        byte_buf->allocated_size += bytes_size + 32;
    }

    memcpy(&(byte_buf->buf[byte_buf->length]), bytes, bytes_size);
    byte_buf->length += bytes_size;

    return 0;
}


int byte_buf_append_byte_buf (struct byte_buf * byte_buf,
                              const struct byte_buf * src) {
    return byte_buf_append_bytes(byte_buf, src->buf, src->length);
}


size_t byte_buf_size (const struct byte_buf * byte_buf) {
    return byte_buf->length;
}


const uint8_t * byte_buf_bytes (const struct byte_buf * byte_buf) {
    return byte_buf->buf;
}