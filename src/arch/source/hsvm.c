#include "arch/source/hsvm.h"

#include "bt/btins.h"

#include <stdio.h>

struct hsvm_register {
    unsigned int value;
    const char * identifier;
};

const struct hsvm_register hsvm_registers[] = {
    {0x0, "r0"},
    {0x1, "r1"},
    {0x2, "r2"},
    {0x3, "r3"},
    {0x4, "r4"},
    {0x5, "r5"},
    {0x6, "r6"},
    {0xA, "r7"},
    {0x8, "rbp"},
    {0x9, "rsp"},
    {0x7, "rip"},
    {-1, NULL}
};

#define OP_ADD      0x10
#define OP_ADDLVAL  0x11
#define OP_SUB      0x12
#define OP_SUBLVAL  0x13
#define OP_MUL      0x14
#define OP_MULLVAL  0x15
#define OP_DIV      0x16
#define OP_DIVLVAL  0x17
#define OP_MOD      0x18
#define OP_MODLVAL  0x19
#define OP_AND      0x1A
#define OP_ANDLVAL  0x1B
#define OP_OR       0x1C
#define OP_ORLVAL   0x1D
#define OP_XOR      0x1E
#define OP_XORLVAL  0x1F
#define OP_JMP      0x20
#define OP_JE       0x21
#define OP_JNE      0x22
#define OP_JL       0x23
#define OP_JLE      0x24
#define OP_JG       0x25
#define OP_JGE      0x26
#define OP_CALL     0x27
#define OP_CALLR    0x28
#define OP_RET      0x29
#define OP_LOAD     0x30
#define OP_LOADR    0x31
#define OP_LOADB    0x32
#define OP_LOADBR   0x33
#define OP_STOR     0x34
#define OP_STORR    0x35
#define OP_STORB    0x36
#define OP_STORBR   0x37
#define OP_IN       0x40
#define OP_OUT      0x41
#define OP_PUSH     0x42
#define OP_PUSHLVAL 0x43
#define OP_POP      0x44
#define OP_MOV      0x51
#define OP_MOVLVAL  0x52
#define OP_CMP      0x53
#define OP_CMPLVAL  0x54
#define OP_HLT      0x60
#define OP_SYSCALL  0x61
#define OP_NOP      0x90


struct hsvm_op {
    int opcode;
    int encoding;
};

enum {
    ENCODING_A,
    ENCODING_B,
    ENCODING_C,
    ENCODING_D,
    ENCODING_E,
    ENCODING_F
};

struct hsvm_op hsvm_ops [] = {
    {OP_JMP,      ENCODING_F},
    {OP_JE,       ENCODING_F},
    {OP_JLE,      ENCODING_F},
    {OP_JG,       ENCODING_F},
    {OP_JGE,      ENCODING_F},
    {OP_CALL,     ENCODING_F},
    {OP_CALLR,    ENCODING_B},
    {OP_RET,      ENCODING_A},
    {OP_LOAD,     ENCODING_E},
    {OP_LOADR,    ENCODING_C},
    {OP_LOADB,    ENCODING_E},
    {OP_LOADBR,   ENCODING_C},
    {OP_STOR,     ENCODING_E},
    {OP_STORR,    ENCODING_C},
    {OP_STORB,    ENCODING_E},
    {OP_STORBR,   ENCODING_C},
    {OP_IN,       ENCODING_B},
    {OP_OUT,      ENCODING_C},
    {OP_PUSH,     ENCODING_B},
    {OP_PUSHLVAL, ENCODING_F},
    {OP_POP,      ENCODING_B},
    {OP_MOV,      ENCODING_C},
    {OP_MOVLVAL,  ENCODING_E},
    {OP_CMP,      ENCODING_C},
    {OP_CMPLVAL,  ENCODING_E},
    {OP_HLT,      ENCODING_A},
    {OP_SYSCALL,  ENCODING_A},
    {OP_NOP,      ENCODING_A},
    {-1, -1}
};


struct boper * hsvm_register (unsigned int r) {
    int i;
    for (i = 0; hsvm_registers[i].value != -1; i++) {
        if (hsvm_registers[i].value == r)
            return boper_variable(16, hsvm_registers[i].identifier);
    }
    return NULL;
}


struct list * hsvm_translate_arith (const uint8_t * u8buf, size_t size) {
    if (size < 4)
        return NULL;

    struct boper * dst = hsvm_register(u8buf[1]);
    struct boper * lhs = NULL;
    struct boper * rhs = NULL;
    if (u8buf[0] & 1) {
        lhs = OCOPY(dst);
        rhs = boper_constant(16, (u8buf[2] << 8) | u8buf[3]);
    }
    else {
        lhs = hsvm_register(u8buf[2]);
        rhs = hsvm_register(u8buf[3]);
    }

    if ((dst == NULL) || (lhs == NULL) || (rhs == NULL)) {
        if (dst) ODEL(dst);
        if (lhs) ODEL(lhs);
        if (rhs) ODEL(rhs);
        return NULL;
    }

    struct list * list = list_create();

    switch (u8buf[0] & 0x1e) {
    case 0x10 : list_append_(list, bins_add_(dst, lhs, rhs)); break;
    case 0x12 : list_append_(list, bins_sub_(dst, lhs, rhs)); break;
    case 0x14 : list_append_(list, bins_umul_(dst, lhs, rhs)); break;
    case 0x16 : list_append_(list, bins_udiv_(dst, lhs, rhs)); break;
    case 0x18 : list_append_(list, bins_umod_(dst, lhs, rhs)); break;
    case 0x1A : list_append_(list, bins_and_(dst, lhs, rhs)); break;
    case 0x1C : list_append_(list, bins_or_(dst, lhs, rhs)); break;
    case 0x1E : list_append_(list, bins_xor_(dst, lhs, rhs)); break;
    default :
        ODEL(list);
        return NULL;
    }

    list_append_(list, bins_add_(boper_variable(16, "rip"),
                                 boper_variable(16, "rip"),
                                 boper_constant(16, 4)));

    return list;
}


struct list * hsvm_store16 (const struct boper * address,
                                 const struct boper * value) {
    struct list * list = list_create();
    // store high byte
    list_append_(list, bins_shr_(boper_variable(16, "t16"),
                                 OCOPY(value),
                                 boper_constant(16, 8)));
    list_append_(list, bins_trun_(boper_variable(8, "t8"),
                                  boper_variable(16, "t16")));
    list_append_(list, bins_store_(OCOPY(address), boper_variable(8, "t8")));
    // store low byte
    list_append_(list, bins_and_(boper_variable(16, "t16"),
                                 OCOPY(value),
                                 boper_constant(16, 0xff)));
    list_append_(list, bins_trun_(boper_variable(8, "t8"), boper_variable(16, "t16")));
    list_append_(list, bins_add_(boper_variable(16, "t16"),
                                 OCOPY(address),
                                 boper_constant(16, 1)));
    list_append_(list, bins_store_(boper_variable(16, "t16"),
                                   boper_variable(8, "t8")));
    return list;
}


struct list * hsvm_load16 (const struct boper * address,
                           const struct boper * dst) {
    struct list * list = list_create();
    // load high byte
    list_append_(list, bins_load_(boper_variable(8, "t8"),
                                  OCOPY(address)));
    list_append_(list, bins_zext_(boper_variable(16, "t16"),
                                  boper_variable(8, "t8")));
    list_append_(list, bins_shl_(OCOPY(dst),
                                 boper_variable(16, "16"),
                                 boper_constant(8, 8)));
    // load low byte
    list_append_(list, bins_add_(boper_variable(16, "t16"),
                                 OCOPY(address),
                                 boper_constant(16, 1)));
    list_append_(list, bins_load_(boper_variable(8, "t8"),
                                  boper_variable(16, "t16")));
    list_append_(list, bins_zext_(boper_variable(16, "t16"),
                                  boper_variable(8, "t8")));
    list_append_(list, bins_or_(OCOPY(dst),
                                OCOPY(dst),
                                boper_variable(16, "t16")));
    return list;
}


struct list * hsvm_in (const struct boper * dst) {
    static unsigned int in_ctr = 0;

    char variable_name[32];
    snprintf(variable_name, 32, "in_%x", in_ctr++);

    struct list * list = list_create();
    list_append_(list, bins_zext_(OCOPY(dst), boper_variable(8, variable_name)));
    return list;
}


struct list * hsvm_out (const struct boper * src) {
    static unsigned int out_ctr = 0;

    char variable_name[32];
    snprintf(variable_name, 32, "out_%x", out_ctr++);

    struct list * list = list_create();
    list_append_(list, bins_trun_(boper_variable(8, variable_name), OCOPY(src)));
    return list;
}


struct list * hsvm_translate_ins (const void * buf, size_t size) {
    const uint8_t * u8buf = (const uint8_t *) buf;

    if (size < 1)
        return NULL;

    // handle all arith instructions
    if ((u8buf[0] & 0xf0) == 0x10)
        return hsvm_translate_arith(u8buf, size);

    // get the instruction encoding
    int i;
    int encoding = -1;
    for (i = 0; hsvm_ops[i].opcode != -1; i++) {
        if (hsvm_ops[i].opcode == u8buf[0]) {
            encoding = hsvm_ops[i].encoding;
            break;
        }
    }
    if (encoding == -1)
        return NULL;

    // set operands
    struct boper * ra = NULL;
    struct boper * rb = NULL;
    struct boper * rc = NULL;
    struct boper * lval = NULL;

    if (encoding == ENCODING_B) {
        ra = hsvm_register(u8buf[1]);
        if (ra == NULL)
            return NULL;
    }
    else if (encoding == ENCODING_C) {
        ra = hsvm_register(u8buf[1]);
        rb = hsvm_register(u8buf[2]);
        if ((ra == NULL) || (rb == NULL)) {
            if (ra) ODEL(ra);
            if (rb) ODEL(rb);
            return NULL;
        }
    }
    else if (encoding == ENCODING_D) {
        ra = hsvm_register(u8buf[1]);
        rb = hsvm_register(u8buf[2]);
        rc = hsvm_register(u8buf[3]);
        if ((ra == NULL) || (rb == NULL) || (rc == NULL)) {
            if (ra) ODEL(ra);
            if (rb) ODEL(rb);
            if (rc) ODEL(rc);
            return NULL;
        }
    }
    else if (encoding == ENCODING_E) {
        ra = hsvm_register(u8buf[1]);
        lval = boper_constant(16, (u8buf[2] << 8) | u8buf[3]);
        if ((ra == NULL) || (lval == NULL)) {
            if (ra == NULL) ODEL(ra);
            if (lval == NULL) ODEL(lval);
            return NULL;
        }
    }
    else if (encoding == ENCODING_F) {
        lval = boper_constant(16, (u8buf[2] << 8) | u8buf[3]);
        if (lval == NULL)
            return NULL;
    }

    struct list * list = list_create();

    // at this point, opcode and operands should all be valid

    // non-conditional jump
    if (u8buf[0] == OP_JMP) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     lval));
        lval = NULL;
    }
    // all of our conditional jumps
    else if (    (u8buf[0] == OP_JE)
              || (u8buf[0] == OP_JL)
              || (u8buf[0] == OP_JLE)
              || (u8buf[0] == OP_JG)
              || (u8buf[0] == OP_JGE)) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        if (u8buf[0] == OP_JE)
            list_append_(list, bins_cmpeq_(boper_variable(1, "t1"),
                                           boper_variable(16, "flags"),
                                           boper_constant(16, 0)));
        else if (u8buf[0] == OP_JL)
            list_append_(list, bins_cmpltu_(boper_variable(1, "t1"),
                                            boper_variable(16, "flags"),
                                            boper_constant(16, 0)));
        else if (u8buf[0] == OP_JLE)
            list_append_(list, bins_cmpleu_(boper_variable(1, "t1"),
                                            boper_variable(16, "flags"),
                                            boper_constant(16, 0)));
        else if (u8buf[0] == OP_JG)
            list_append_(list, bins_cmpltu_(boper_variable(1, "t1"),
                                            boper_constant(16, 0),
                                            boper_variable(16, "flags")));
        else if (u8buf[0] == OP_JGE)
            list_append_(list, bins_cmpleu_(boper_variable(1, "t1"),
                                            boper_constant(16, 0),
                                            boper_variable(16, "flags")));
        list_append_(list, bins_zext_(boper_variable(16, "t32"),
                                      boper_variable(1, "t1")));
        list_append_(list, bins_umul_(boper_variable(16, "t32"),
                                      lval,
                                      boper_variable(16, "t32")));
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_variable(16, "t32")));
        lval = NULL;
    }
    // call, callr
    else if ((u8buf[0] == OP_CALL) || (u8buf[0] == OP_CALLR)) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_sub_(boper_variable(16, "rsp"),
                                     boper_variable(16, "rsp"),
                                     boper_constant(16, 2)));
        struct boper * address = boper_variable(16, "rsp");
        struct boper * value = boper_variable(16, "rip");
        struct list * store_list = hsvm_store16(address, value);
        list_append_list(list, store_list);
        ODEL(store_list);
        ODEL(value);
        ODEL(address);
        if (u8buf[0] == OP_CALL) {
            list_append_(list, bins_add_(boper_variable(16, "rsp"),
                                         boper_variable(16, "rsp"),
                                         lval));
            lval = NULL;
        }
        else {
            list_append_(list, bins_add_(boper_variable(16, "rsp"),
                                         boper_variable(16, "rsp"),
                                         ra));
            ra = NULL;
        }
    }
    // ret
    else if (u8buf[0] == OP_RET) {
        struct boper * address = boper_variable(16, "rsp");
        struct boper * dst = boper_variable(16, "rip");
        struct list * load_list = hsvm_load16(address, dst);
        list_append_list(list, load_list);
        ODEL(load_list);
        ODEL(dst);
        ODEL(address);
        list_append_(list, bins_add_(boper_variable(16, "rsp"),
                                     boper_variable(16, "rsp"),
                                     boper_constant(16, 2)));
    }
    // load
    else if (u8buf[0] == OP_LOAD) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        struct list * load_list = hsvm_load16(lval, ra);
        list_append_list(list, load_list);
        ODEL(load_list);
    }
    // loadr
    else if (u8buf[0] == OP_LOADR) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        struct list * load_list = hsvm_load16(rb, ra);
        list_append_list(list, load_list);
        ODEL(load_list);
    }
    // loadb
    else if (u8buf[0] == OP_LOADB) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_load_(boper_variable(8, "t8"), lval));
        list_append_(list, bins_zext_(ra, boper_variable(8, "t8")));
        ra = NULL;
        lval = NULL;
    }
    // loadbr
    else if (u8buf[0] == OP_LOADBR) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_load_(boper_variable(8, "t8"), rb));
        list_append_(list, bins_zext_(ra, boper_variable(8, "t8")));
        ra = NULL;
        rb = NULL;
    }
    else if (u8buf[0] == OP_STOR) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        struct list * store_list = hsvm_store16(lval, ra);
        list_append_list(list, store_list);
        ODEL(store_list);
    }
    else if (u8buf[0] == OP_STORR) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        struct list * store_list = hsvm_store16(rb, ra);
        list_append_list(list, store_list);
        ODEL(store_list);
    }
    else if (u8buf[0] == OP_STORB) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_trun_(boper_variable(8, "t8"), ra));
        list_append_(list, bins_store_(lval, boper_variable(8, "t8")));
        ra = NULL;
        lval = NULL;
    }
    else if (u8buf[0] == OP_STORBR) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_trun_(boper_variable(8, "t8"), ra));
        list_append_(list, bins_store_(rb, boper_variable(8, "t8")));
        ra = NULL;
        rb = NULL;
    }
    else if (u8buf[0] == OP_IN) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        struct list * in_list = hsvm_in(ra);
        list_append_list(list, in_list);
        ODEL(in_list);
    }
    else if (u8buf[0] == OP_OUT) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        struct list * out_list = hsvm_out(ra);
        list_append_list(list, out_list);
        ODEL(out_list);
    }
    else if (u8buf[0] == OP_PUSH) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_sub_(boper_variable(16, "rsp"),
                                     boper_variable(16, "rsp"),
                                     boper_constant(16, 2)));
        struct boper * rsp = boper_variable(16, "rsp");
        struct list * store_list = hsvm_store16(rsp, ra);
        list_append_list(list, store_list);
        ODEL(store_list);
        ODEL(rsp);
    }
    else if (u8buf[0] == OP_PUSHLVAL) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_sub_(boper_variable(16, "rsp"),
                                     boper_variable(16, "rsp"),
                                     boper_constant(16, 2)));
        struct boper * rsp = boper_variable(16, "rsp");
        struct list * store_list = hsvm_store16(rsp, lval);
        list_append_list(list, store_list);
        ODEL(store_list);
        ODEL(rsp);
    }
    else if (u8buf[0] == OP_POP) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        struct boper * rsp = boper_variable(16, "rsp");
        struct list * load_list = hsvm_load16(rsp, ra);
        list_append_list(list, load_list);
        ODEL(load_list);
        ODEL(rsp);
        list_append_(list, bins_add_(boper_variable(16, "rsp"),
                                     boper_variable(16, "rsp"),
                                     boper_constant(16, 2)));
    }
    else if (u8buf[0] == OP_MOV) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_or_(ra, rb, boper_constant(16, 0)));
        ra = NULL;
        rb = NULL;
    }
    else if (u8buf[0] == OP_MOVLVAL) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_or_(ra, lval, boper_constant(16, 0)));
        ra = NULL;
        lval = NULL;
    }
    else if (u8buf[0] == OP_CMP) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_sub_(boper_variable(16, "flags"), ra, rb));
    }
    else if (u8buf[0] == OP_CMPLVAL) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_sub_(boper_variable(16, "flags"), ra, lval));
    }
    else if (u8buf[0] == OP_HLT) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_hlt());
    }
    else if (u8buf[0] == OP_NOP) {
        list_append_(list, bins_add_(boper_variable(16, "rip"),
                                     boper_variable(16, "rip"),
                                     boper_constant(16, 4)));
        list_append_(list, bins_or_(boper_constant(16, 0),
                                    boper_constant(16, 0),
                                    boper_constant(16, 0)));
    }

    if (ra) ODEL(ra);
    if (rb) ODEL(rb);
    if (rc) ODEL(rc);
    if (lval) ODEL(lval);

    return list;
}


struct list * hsvm_translate_block (const void * buf, size_t size) {
    const uint8_t * u8buf = (const uint8_t *) buf;

    struct list * list = list_create();
    size_t offset;
    for (offset = 0; offset + 4 <= size; offset += 4) {
        struct list * ins_list = hsvm_translate_ins(&(u8buf[offset]), 4);
        if (ins_list == NULL) {
            ODEL(list);
            return NULL;
        }
        list_append_list(list, ins_list);
        ODEL(ins_list);

        int break_loop = 0;
        switch (u8buf[offset]) {
        case OP_JMP :
        case OP_JE :
        case OP_JNE :
        case OP_JL :
        case OP_JLE :
        case OP_JGE :
        case OP_CALL :
        case OP_CALLR :
        case OP_RET :
        case OP_HLT :
        case OP_SYSCALL :
            break_loop = 1;
            break;
        }
        if (break_loop)
            break;
    }

    return list;
}