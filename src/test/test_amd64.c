#include "arch/target/amd64.h"
#include "bt/bins.h"
#include "container/byte_buf.h"
#include "container/list.h"
#include "container/varstore.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#define TEST_ITERATIONS 4


/* Executable memory for copying and running assembled code. */
void * mmap_mem;

/* Length of data in mmap_mem, just in case we need to dump it out for debugging
   purposes. */
size_t mmap_length;


int dump_mmap_mem () {
    FILE * fh = fopen("/tmp/mmap_mem", "wb");
    fwrite(mmap_mem, 1, mmap_length, fh);
    fclose(fh);
    return 0;
}


/*
* Helpers for creating and working with lhs/rhs operators at different bit
* widths.
*/
struct arithmetic_operands {
    uint8_t result8;
    uint8_t l8;
    uint8_t r8;
    uint16_t result16;
    uint16_t l16;
    uint16_t r16;
    uint32_t result32;
    uint32_t l32;
    uint32_t r32;
    uint64_t result64;
    uint64_t l64;
    uint64_t r64;
};


int arithmetic_operands_random (struct arithmetic_operands * ao) {
    FILE * fh = fopen("/dev/urandom", "rb");
    fread(ao, 1, sizeof(struct arithmetic_operands), fh);
    fclose(fh);

    if (ao->r8 == 0)
        ao->r8++;
    else if (ao->r16 == 0)
        ao->r16++;
    else if (ao->r32 == 0)
        ao->r32++;
    else if (ao->r64 == 0)
        ao->r64++;
    return 0;
}


struct list * arithmetic_operands_set (struct arithmetic_operands * ao) {
    struct list * list = list_create();

    list_append(list, bins_or_(boper_variable(8, "l8"),
                               boper_constant(8, 0),
                               boper_constant(8, ao->l8)));
    list_append(list, bins_or_(boper_variable(8, "r8"),
                               boper_constant(8, 0),
                               boper_constant(8, ao->r8)));
    list_append(list, bins_or_(boper_variable(16, "l16"),
                               boper_constant(16, 0),
                               boper_constant(16, ao->l16)));
    list_append(list, bins_or_(boper_variable(16, "r16"),
                               boper_constant(16, 0),
                               boper_constant(16, ao->r16)));
    list_append(list, bins_or_(boper_variable(32, "l32"),
                               boper_constant(32, 0),
                               boper_constant(32, ao->l32)));
    list_append(list, bins_or_(boper_variable(32, "r32"),
                               boper_constant(32, 0),
                               boper_constant(32, ao->r32)));
    list_append(list, bins_or_(boper_variable(64, "l64"),
                               boper_constant(64, 0),
                               boper_constant(64, ao->l64)));
    list_append(list, bins_or_(boper_variable(64, "r64"),
                               boper_constant(64, 0),
                               boper_constant(64, ao->r64)));
    return list;
}


/*
* Test functions for arithmetic instructions
*/


int test_arith (
    struct arithmetic_operands * ao,
    struct bins * (* op_) (struct boper *, struct boper *, struct boper *)
) {
    arithmetic_operands_random(ao);
    struct list * list = arithmetic_operands_set(ao);

    list_append_(list, op_(boper_variable(8, "result8"),
                           boper_variable(8, "l8"),
                           boper_variable(8, "r8")));
    list_append_(list, op_(boper_variable(16, "result16"),
                           boper_variable(16, "l16"),
                           boper_variable(16, "r16")));
    list_append_(list, op_(boper_variable(32, "result32"),
                           boper_variable(32, "l32"),
                           boper_variable(32, "r32")));
    list_append_(list, op_(boper_variable(64, "result64"),
                           boper_variable(64, "l64"),
                           boper_variable(64, "r64")));
    struct varstore * varstore = varstore_create();

    struct byte_buf * assembled = amd64_assemble(list, varstore);

    memcpy(mmap_mem, byte_buf_bytes(assembled), byte_buf_length(assembled));
    mmap_length = byte_buf_length(assembled);

    dump_mmap_mem();
    assert(amd64_execute(mmap_mem, varstore) == 0);

    uint64_t result;
    assert(varstore_value(varstore, "result8", 8, &result) == 0);
    ao->result8 = result;
    assert(varstore_value(varstore, "result16", 16, &result) == 0);
    ao->result16 = result;
    assert(varstore_value(varstore, "result32", 32, &result) == 0);
    ao->result32 = result;
    assert(varstore_value(varstore, "result64", 64, &result) == 0);
    ao->result64 = result;

    ODEL(list);
    ODEL(varstore);
    ODEL(assembled);

    return 0;
}


int test_add () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_arith(&ao, &bins_add_);

        if (ao.result8  != (uint8_t) (ao.l8 + ao.r8)) {
            printf("add 8 (0x%02x = 0x%02x + 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 + ao.r16)) {
            printf("add 16 (0x%04x = 0x%04x + 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 + ao.r32)) {
            printf("add 32 (0x%08x = 0x%08x + 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 + ao.r64)) {
            printf("add 64 (0x%016llx = 0x%016llx + 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


int test_sub () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_arith(&ao, &bins_sub_);

        if (ao.result8  != (uint8_t) (ao.l8 - ao.r8)) {
            printf("sub 8 (0x%02x = 0x%02x - 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 - ao.r16)) {
            printf("sub 16 (0x%04x = 0x%04x - 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 - ao.r32)) {
            printf("sub 32 (0x%08x = 0x%08x - 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 - ao.r64)) {
            printf("sub 64 (0x%016llx = 0x%016llx - 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


int test_umul () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_arith(&ao, &bins_umul_);

        if (ao.result8  != (uint8_t) (ao.l8 * ao.r8)) {
            printf("umul 8 (0x%02x = 0x%02x * 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 * ao.r16)) {
            printf("umul 16 (0x%04x = 0x%04x * 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 * ao.r32)) {
            printf("umul 32 (0x%08x = 0x%08x * 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 * ao.r64)) {
            printf("umul 64 (0x%016llx = 0x%016llx * 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


int test_udiv () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_arith(&ao, &bins_udiv_);

        if (ao.result8  != (uint8_t) (ao.l8 / ao.r8)) {
            printf("udiv 8 (0x%02x = 0x%02x / 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 / ao.r16)) {
            printf("udiv 16 (0x%04x = 0x%04x / 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 / ao.r32)) {
            printf("udiv 32 (0x%08x = 0x%08x / 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 / ao.r64)) {
            printf("udiv 64 (0x%016llx = 0x%016llx / 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


int test_umod () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_arith(&ao, &bins_umod_);

        if (ao.result8  != (uint8_t) (ao.l8 % ao.r8)) {
            printf("umod 8 (0x%02x = 0x%02x %% 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 % ao.r16)) {
            printf("umod 16 (0x%04x = 0x%04x %% 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 % ao.r32)) {
            printf("umod 32 (0x%08x = 0x%08x %% 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 % ao.r64)) {
            printf("umod 64 (0x%016llx = 0x%016llx %% 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


int test_and () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_arith(&ao, &bins_and_);

        if (ao.result8  != (uint8_t) (ao.l8 & ao.r8)) {
            printf("and 8 (0x%02x = 0x%02x & 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 & ao.r16)) {
            printf("and 16 (0x%04x = 0x%04x & 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 & ao.r32)) {
            printf("and 32 (0x%08x = 0x%08x & 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 & ao.r64)) {
            printf("and 64 (0x%016llx = 0x%016llx & 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


int test_or () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_arith(&ao, &bins_or_);

        if (ao.result8  != (uint8_t) (ao.l8 | ao.r8)) {
            printf("or 8 (0x%02x = 0x%02x | 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 | ao.r16)) {
            printf("or 16 (0x%04x = 0x%04x | 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 | ao.r32)) {
            printf("or 32 (0x%08x = 0x%08x | 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 | ao.r64)) {
            printf("or 64 (0x%016llx = 0x%016llx | 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


int test_xor () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_arith(&ao, &bins_xor_);

        if (ao.result8  != (uint8_t) (ao.l8 ^ ao.r8)) {
            printf("xor 8 (0x%02x = 0x%02x ^ 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 ^ ao.r16)) {
            printf("xor 16 (0x%04x = 0x%04x ^ 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 ^ ao.r32)) {
            printf("xor 32 (0x%08x = 0x%08x ^ 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 ^ ao.r64)) {
            printf("xor 64 (0x%016llx = 0x%016llx ^ 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


/*
* Shifts are almost identical to normal instructions, except we gaurantee the
* width of the shift will fit within the width of the destination variable.
*/
int test_shift (
    struct arithmetic_operands * ao,
    struct bins * (* op_) (struct boper *, struct boper *, struct boper *)
) {
    arithmetic_operands_random(ao);

    ao->r8 &= 0x7;
    ao->r16 &= 0xf;
    ao->r32 &= 0x1f;
    ao->r64 &= 0x3f;

    struct list * list = arithmetic_operands_set(ao);

    list_append_(list, op_(boper_variable(8, "result8"),
                           boper_variable(8, "l8"),
                           boper_variable(8, "r8")));
    list_append_(list, op_(boper_variable(16, "result16"),
                           boper_variable(16, "l16"),
                           boper_variable(16, "r16")));
    list_append_(list, op_(boper_variable(32, "result32"),
                           boper_variable(32, "l32"),
                           boper_variable(32, "r32")));
    list_append_(list, op_(boper_variable(64, "result64"),
                           boper_variable(64, "l64"),
                           boper_variable(64, "r64")));
    struct varstore * varstore = varstore_create();

    struct byte_buf * assembled = amd64_assemble(list, varstore);

    memcpy(mmap_mem, byte_buf_bytes(assembled), byte_buf_length(assembled));
    mmap_length = byte_buf_length(assembled);

    dump_mmap_mem();
    assert(amd64_execute(mmap_mem, varstore) == 0);

    uint64_t result;
    assert(varstore_value(varstore, "result8", 8, &result) == 0);
    ao->result8 = result;
    assert(varstore_value(varstore, "result16", 16, &result) == 0);
    ao->result16 = result;
    assert(varstore_value(varstore, "result32", 32, &result) == 0);
    ao->result32 = result;
    assert(varstore_value(varstore, "result64", 64, &result) == 0);
    ao->result64 = result;

    ODEL(list);
    ODEL(varstore);
    ODEL(assembled);

    return 0;
}


int test_shl () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_shift(&ao, &bins_shl_);

        if (ao.result8  != (uint8_t) (ao.l8 << ao.r8)) {
            printf("shl 8 (0x%02x = 0x%02x << 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 << ao.r16)) {
            printf("shl 16 (0x%04x = 0x%04x << 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 << ao.r32)) {
            printf("shl 32 (0x%08x = 0x%08x << 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 << ao.r64)) {
            printf("shl 64 (0x%016llx = 0x%016llx << 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


int test_shr () {
    unsigned int i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        struct arithmetic_operands ao;
        test_shift(&ao, &bins_shr_);

        if (ao.result8  != (uint8_t) (ao.l8 >> ao.r8)) {
            printf("shr 8 (0x%02x = 0x%02x >> 0x%02x)\n",
                   ao.result8, ao.l8, ao.r8);
            return -1;
        }
        else if (ao.result16 != (uint16_t) (ao.l16 >> ao.r16)) {
            printf("shr 16 (0x%04x = 0x%04x >> 0x%04x)\n",
                   ao.result16, ao.l16, ao.r16);
            return -1;
        }
        else if (ao.result32 != (uint32_t) (ao.l32 >> ao.r32)) {
            printf("shr 32 (0x%08x = 0x%08x >> 0x%08x)\n",
                   ao.result32, ao.l32, ao.r32);
            return -1;
        }
        else if (ao.result64 != (uint64_t) (ao.l64 >> ao.r64)) {
            printf("shr 64 (0x%016llx = 0x%016llx >> 0x%016llx)\n",
                   ao.result64, ao.l64, ao.r64);
            return -1;
        }
    }

    return 0;
}


/*
* We're going to essentially borrow arithmetic stuff for comparison
* instructions.
*/
int test_comparison (
    struct arithmetic_operands * ao,
    struct bins * (* op_) (struct boper *, struct boper *, struct boper *)
) {
    struct list * list = arithmetic_operands_set(ao);

    list_append_(list, op_(boper_variable(1, "result8"),
                           boper_variable(8, "l8"),
                           boper_variable(8, "r8")));
    list_append_(list, op_(boper_variable(1, "result16"),
                           boper_variable(16, "l16"),
                           boper_variable(16, "r16")));
    list_append_(list, op_(boper_variable(1, "result32"),
                           boper_variable(32, "l32"),
                           boper_variable(32, "r32")));
    list_append_(list, op_(boper_variable(1, "result64"),
                           boper_variable(64, "l64"),
                           boper_variable(64, "r64")));
    struct varstore * varstore = varstore_create();

    struct byte_buf * assembled = amd64_assemble(list, varstore);

    memcpy(mmap_mem, byte_buf_bytes(assembled), byte_buf_length(assembled));
    mmap_length = byte_buf_length(assembled);

    dump_mmap_mem();
    assert(amd64_execute(mmap_mem, varstore) == 0);

    uint64_t result;
    assert(varstore_value(varstore, "result8", 1, &result) == 0);
    ao->result8 = result;
    assert(varstore_value(varstore, "result16", 1, &result) == 0);
    ao->result16 = result;
    assert(varstore_value(varstore, "result32", 1, &result) == 0);
    ao->result32 = result;
    assert(varstore_value(varstore, "result64", 1, &result) == 0);
    ao->result64 = result;

    ODEL(list);
    ODEL(varstore);
    ODEL(assembled);

    return 0;
}


int test_cmpeq_ (struct arithmetic_operands * ao) {
    test_comparison(ao, &bins_cmpeq_);

    if (ao->result8 != ao->l8 == ao->r8 ? 1 : 0) {
        printf("cmpeq 8 (0x%02x = 0x%02x == 0x%02x)\n",
               ao->result8, ao->l8, ao->r8);
        return -1;
    }
    else if (ao->result16 != ao->l16 == ao->r16 ? 1 : 0) {
        printf("cmpeq 16 (0x%04x = 0x%04x == 0x%04x)\n",
               ao->result16, ao->l16, ao->r16);
        return -1;
    }
    else if (ao->result32 != ao->l32 == ao->r32 ? 1 : 0) {
        printf("cmpeq 32 (0x%08x = 0x%08x == 0x%08x)\n",
               ao->result32, ao->l32, ao->r32);
        return -1;
    }
    else if (ao->result64 != ao->l64 == ao->r64 ? 1 : 0) {
        printf("cmpeq 64 (0x%016llx = 0x%016llx == 0x%016llx)\n",
               ao->result64, ao->l64, ao->r64);
        return -1;
    }

    return 0;
}


int test_cmpeq () {
    struct arithmetic_operands ao;

    ao.l8 = 0x12;
    ao.r8 = 0x12;
    ao.l16 = 0x1234;
    ao.r16 = 0x1234;
    ao.l32 = 0x12345678;
    ao.r32 = 0x12345678;
    ao.l64 = 0x0123456789abcdef;
    ao.r64 = 0x0123456789abcdef;

    if (test_cmpeq_(&ao))
        return -1;

    unsigned i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        arithmetic_operands_random(&ao);
        if (test_cmpeq_(&ao))
            return -1;
    }

    return 0;
}


int test_cmpltu_ (struct arithmetic_operands * ao) {
    test_comparison(ao, &bins_cmpltu_);

    if (ao->result8 != ao->l8 < ao->r8 ? 1 : 0) {
        printf("cmpltu 8 (0x%02x = 0x%02x < 0x%02x)\n",
               ao->result8, ao->l8, ao->r8);
        return -1;
    }
    else if (ao->result16 != ao->l16 < ao->r16 ? 1 : 0) {
        printf("cmpltu 16 (0x%04x = 0x%04x < 0x%04x)\n",
               ao->result16, ao->l16, ao->r16);
        return -1;
    }
    else if (ao->result32 != ao->l32 < ao->r32 ? 1 : 0) {
        printf("cmpltu 32 (0x%08x = 0x%08x < 0x%08x)\n",
               ao->result32, ao->l32, ao->r32);
        return -1;
    }
    else if (ao->result64 != ao->l64 < ao->r64 ? 1 : 0) {
        printf("cmpltu 64 (0x%016llx = 0x%016llx < 0x%016llx)\n",
               ao->result64, ao->l64, ao->r64);
        return -1;
    }

    return 0;
}


int test_cmpltu () {
    struct arithmetic_operands ao;

    ao.l8 = 0x12;
    ao.r8 = 0x12;
    ao.l16 = 0x1234;
    ao.r16 = 0x1234;
    ao.l32 = 0x12345678;
    ao.r32 = 0x12345678;
    ao.l64 = 0x0123456789abcdef;
    ao.r64 = 0x0123456789abcdef;

    if (test_cmpltu_(&ao))
        return -1;

    ao.l8 = 0x12;
    ao.r8 = 0x13;
    ao.l16 = 0x1234;
    ao.r16 = 0x1235;
    ao.l32 = 0x12345678;
    ao.r32 = 0x12345679;
    ao.l64 = 0x0123456789abcdef;
    ao.r64 = 0x0123456789abcdf0;

    if (test_cmpltu_(&ao))
        return -1;

    ao.l8 = 0x13;
    ao.r8 = 0x12;
    ao.l16 = 0x1235;
    ao.r16 = 0x1234;
    ao.l32 = 0x12345679;
    ao.r32 = 0x12345678;
    ao.l64 = 0x0123456789abcdf0;
    ao.r64 = 0x0123456789abcdef;

    if (test_cmpltu_(&ao))
        return -1;

    unsigned i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        arithmetic_operands_random(&ao);
        if (test_cmpltu_(&ao))
            return -1;
    }

    return 0;
}


int test_cmplts_ (struct arithmetic_operands * ao) {
    test_comparison(ao, &bins_cmplts_);

    if (ao->result8 != (int8_t) ao->l8 < (int8_t) ao->r8 ? 1 : 0) {
        printf("cmplts 8 (0x%02x = 0x%02x < 0x%02x)\n",
               ao->result8, ao->l8, ao->r8);
        return -1;
    }
    else if (ao->result16 != (int16_t) ao->l16 < (int16_t) ao->r16 ? 1 : 0) {
        printf("cmplts 16 (0x%04x = 0x%04x < 0x%04x)\n",
               ao->result16, ao->l16, ao->r16);
        return -1;
    }
    else if (ao->result32 != (int32_t) ao->l32 < (int32_t) ao->r32 ? 1 : 0) {
        printf("cmplts 32 (0x%08x = 0x%08x < 0x%08x)\n",
               ao->result32, ao->l32, ao->r32);
        return -1;
    }
    else if (ao->result64 != (int64_t) ao->l64 < (int64_t) ao->r64 ? 1 : 0) {
        printf("cmplts 64 (0x%016llx = 0x%016llx < 0x%016llx)\n",
               ao->result64, ao->l64, ao->r64);
        return -1;
    }

    return 0;
}


int test_cmplts () {
    struct arithmetic_operands ao;

    ao.l8 = 0x12;
    ao.r8 = 0x12;
    ao.l16 = 0x1234;
    ao.r16 = 0x1234;
    ao.l32 = 0x12345678;
    ao.r32 = 0x12345678;
    ao.l64 = 0x0123456789abcdef;
    ao.r64 = 0x0123456789abcdef;

    if (test_cmplts_(&ao))
        return -1;

    ao.l8 = 0xf2;
    ao.r8 = 0xf3;
    ao.l16 = 0xf234;
    ao.r16 = 0xf235;
    ao.l32 = 0xf2345678;
    ao.r32 = 0xf2345679;
    ao.l64 = 0xf123456789abcdef;
    ao.r64 = 0xf123456789abcdf0;

    if (test_cmplts_(&ao))
        return -1;

    ao.l8 = 0xf3;
    ao.r8 = 0xf2;
    ao.l16 = 0xf235;
    ao.r16 = 0xf234;
    ao.l32 = 0xf2345679;
    ao.r32 = 0xf2345678;
    ao.l64 = 0xf123456789abcdf0;
    ao.r64 = 0xf123456789abcdef;

    if (test_cmplts_(&ao))
        return -1;

    unsigned i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        arithmetic_operands_random(&ao);
        if (test_cmplts_(&ao))
            return -1;
    }

    return 0;
}


int test_cmpleu_ (struct arithmetic_operands * ao) {
    test_comparison(ao, &bins_cmpleu_);

    if (ao->result8 != ao->l8 <= ao->r8 ? 1 : 0) {
        printf("cmpleu 8 (0x%02x = 0x%02x <= 0x%02x)\n",
               ao->result8, ao->l8, ao->r8);
        return -1;
    }
    else if (ao->result16 != ao->l16 <= ao->r16 ? 1 : 0) {
        printf("cmpleu 16 (0x%04x = 0x%04x <= 0x%04x)\n",
               ao->result16, ao->l16, ao->r16);
        return -1;
    }
    else if (ao->result32 != ao->l32 <= ao->r32 ? 1 : 0) {
        printf("cmpleu 32 (0x%08x = 0x%08x <= 0x%08x)\n",
               ao->result32, ao->l32, ao->r32);
        return -1;
    }
    else if (ao->result64 != ao->l64 <= ao->r64 ? 1 : 0) {
        printf("cmpleu 64 (0x%016llx = 0x%016llx <= 0x%016llx)\n",
               ao->result64, ao->l64, ao->r64);
        return -1;
    }

    return 0;
}


int test_cmpleu () {
    struct arithmetic_operands ao;

    ao.l8 = 0x12;
    ao.r8 = 0x12;
    ao.l16 = 0x1234;
    ao.r16 = 0x1234;
    ao.l32 = 0x12345678;
    ao.r32 = 0x12345678;
    ao.l64 = 0x0123456789abcdef;
    ao.r64 = 0x0123456789abcdef;

    if (test_cmpleu_(&ao))
        return -1;

    ao.l8 = 0x12;
    ao.r8 = 0x13;
    ao.l16 = 0x1234;
    ao.r16 = 0x1235;
    ao.l32 = 0x12345678;
    ao.r32 = 0x12345679;
    ao.l64 = 0x0123456789abcdef;
    ao.r64 = 0x0123456789abcdf0;

    if (test_cmpleu_(&ao))
        return -1;

    ao.l8 = 0x13;
    ao.r8 = 0x12;
    ao.l16 = 0x1235;
    ao.r16 = 0x1234;
    ao.l32 = 0x12345679;
    ao.r32 = 0x12345678;
    ao.l64 = 0x0123456789abcdf0;
    ao.r64 = 0x0123456789abcdef;

    if (test_cmpleu_(&ao))
        return -1;

    unsigned i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        arithmetic_operands_random(&ao);
        if (test_cmpleu_(&ao))
            return -1;
    }

    return 0;
}


int test_cmples_ (struct arithmetic_operands * ao) {
    test_comparison(ao, &bins_cmples_);

    if (ao->result8 != (int8_t) ao->l8 <= (int8_t) ao->r8 ? 1 : 0) {
        printf("cmples 8 (0x%02x = 0x%02x < 0x%02x)\n",
               ao->result8, ao->l8, ao->r8);
        return -1;
    }
    else if (ao->result16 != (int16_t) ao->l16 <= (int16_t) ao->r16 ? 1 : 0) {
        printf("cmples 16 (0x%04x = 0x%04x <= 0x%04x)\n",
               ao->result16, ao->l16, ao->r16);
        return -1;
    }
    else if (ao->result32 != (int32_t) ao->l32 <= (int32_t) ao->r32 ? 1 : 0) {
        printf("cmples 32 (0x%08x = 0x%08x <= 0x%08x)\n",
               ao->result32, ao->l32, ao->r32);
        return -1;
    }
    else if (ao->result64 != (int64_t) ao->l64 <= (int64_t) ao->r64 ? 1 : 0) {
        printf("cmples 64 (0x%016llx = 0x%016llx <= 0x%016llx)\n",
               ao->result64, ao->l64, ao->r64);
        return -1;
    }

    return 0;
}


int test_cmples () {
    struct arithmetic_operands ao;

    ao.l8 = 0x12;
    ao.r8 = 0x12;
    ao.l16 = 0x1234;
    ao.r16 = 0x1234;
    ao.l32 = 0x12345678;
    ao.r32 = 0x12345678;
    ao.l64 = 0x0123456789abcdef;
    ao.r64 = 0x0123456789abcdef;

    if (test_cmples_(&ao))
        return -1;

    ao.l8 = 0xf2;
    ao.r8 = 0xf3;
    ao.l16 = 0xf234;
    ao.r16 = 0xf235;
    ao.l32 = 0xf2345678;
    ao.r32 = 0xf2345679;
    ao.l64 = 0xf123456789abcdef;
    ao.r64 = 0xf123456789abcdf0;

    if (test_cmples_(&ao))
        return -1;

    ao.l8 = 0xf3;
    ao.r8 = 0xf2;
    ao.l16 = 0xf235;
    ao.r16 = 0xf234;
    ao.l32 = 0xf2345679;
    ao.r32 = 0xf2345678;
    ao.l64 = 0xf123456789abcdf0;
    ao.r64 = 0xf123456789abcdef;

    if (test_cmples_(&ao))
        return -1;

    unsigned i;
    for (i = 0; i < TEST_ITERATIONS; i++) {
        arithmetic_operands_random(&ao);
        if (test_cmples_(&ao))
            return -1;
    }

    return 0;
}


int main (int argc, char * argv[]) {
    mmap_mem = mmap(0, 4096 * 16, PROT_READ | PROT_WRITE | PROT_EXEC,
                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (test_add()) {
        printf("error in test_add()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_sub()) {
        printf("error in test_sub()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_umul()) {
        printf("error in test_umul()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_udiv()) {
        printf("error in test_udiv()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_umod()) {
        printf("error in test_umod()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_and()) {
        printf("error in test_and()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_or()) {
        printf("error in test_or()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_xor()) {
        printf("error in test_xor()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_shl()) {
        printf("error in test_shl()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_shr()) {
        printf("error in test_shr()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_cmpeq()) {
        printf("error in test_cmpeq()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_cmpltu()) {
        printf("error in test_cmpltu()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_cmplts()) {
        printf("error in test_cmplts()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_cmpleu()) {
        printf("error in test_cmpleu()\n");
        dump_mmap_mem();
        return -1;
    }
    else if (test_cmples()) {
        printf("error in test_cmples()\n");
        dump_mmap_mem();
        return -1;
    }
    munmap(mmap_mem, 4096 * 16);
    return 0;
}
