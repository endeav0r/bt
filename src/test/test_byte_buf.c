#include "testobj.h"

#include "container/byte_buf.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main () {
    struct byte_buf * bb = byte_buf_create();

    byte_buf_append(bb, 0x01);
    byte_buf_append_le16(bb, 0x0302);
    byte_buf_append_le32(bb, 0x07060504);
    byte_buf_append_le64(bb, 0x0f0e0d0c0b0a0908);

    struct byte_buf * copy = OCOPY(bb);

    uint8_t compare_bytes [] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c,
        0x0d, 0x0e, 0x0f
    };
    unsigned int i;
    for (i = 0; i < 15; i++) {
        assert(byte_buf_bytes(copy)[i] == compare_bytes[i]);
    }

    byte_buf_append_byte_buf(bb, copy);

    assert(byte_buf_length(bb) == 30);

    for (i = 0; i < 30; i++) {
        assert(byte_buf_bytes(bb)[i] == compare_bytes[i % 15]);
    }

    ODEL(bb);
    ODEL(copy);

    return 0;
}