#include "arch/source/x86.h"

#include "bt/bins.h"
#include "container/list.h"

#include <stdlib.h>

#define REG_EAX 0x0
#define REG_ECX 0x1
#define REG_EDX 0x2
#define REG_EBX 0x3
#define REG_ESP 0x4
#define REG_EBP 0x5
#define REG_ESI 0x6
#define REG_EDI 0x7


const struct arch_source arch_source_x86 = {
    asx86_ip_variable_identifier,
    asx86_ip_variable_bits,
    asx86_translate_ins,
    asx86_translate_block
};


const char * asx86_ip_variable_identifier () {
    return "eip";
}


unsigned int asx86_ip_variable_bits () {
    return 32;
}


uint32_t asx86_buf_to_uint32 (const uint8_t * buf) {
    uint32_t r = buf[3] << 24;
    r |= buf[2] << 16;
    r |= buf[1] << 8;
    r |= buf[0];
    return r;
}


uint16_t asx86_buf_to_uint16 (const uint8_t * buf) {
    uint16_t r = buf[1] << 8;
    r |= buf[0];
    return r;
}


enum reg_type {
    REG_TYPE_RL,  /* al */
    REG_TYPE_RH,  /* ah */
    REG_TYPE_RX,  /* ax */
    REG_TYPE_ERX, /* eax */
    REG_TYPE_NS   /* not supported */
};

struct index_string {
    int index;
    reg_type type;
    int full_index;
    const char * string;
};

const struct index_string reg_strings[] = {
    {X86_REG_INVALID, X86_REG_INVALID, NULL},
    {X86_REG_AH,  REG_TYPE_RH,  X86_REG_EAX, "ah"},
    {X86_REG_AL,  REG_TYPE_RL,  X86_REG_EAX, "al"},
    {X86_REG_AX,  REG_TYPE_RX,  X86_REG_EAX, "ax"},
    {X86_REG_BH,  REG_TYPE_RH,  X86_REG_EBX, "bh"},
    {X86_REG_BL,  REG_TYPE_RL,  X86_REG_EBX, "bl"},
    {X86_REG_BP,  REG_TYPE_RX,  X86_REG_EBP, "bp"},
    {X86_REG_BPL, REG_TYPE_RL,  X86_REG_EBP, "bpl"},
    {X86_REG_BX,  REG_TYPE_RX,  X86_REG_EBX, "bx"},
    {X86_REG_CH,  REG_TYPE_RH,  X86_REG_ECX, "ch"},
    {X86_REG_CL,  REG_TYPE_RL,  X86_REG_ECX, "cl"},
    {X86_REG_CS,  REG_TYPE_NS,  X86_REG_CS,  NULL},
    {X86_REG_CX,  REG_TYPE_RX,  X86_REG_ECX, "cx"},
    {X86_REG_DH,  REG_TYPE_RH,  X86_REG_EDX, "dh"},
    {X86_REG_DI,  REG_TYPE_RX,  X86_REG_EDI, "di"},
    {X86_REG_DIL, REG_TYPE_RL,  X86_REG_EDI, "dil"},
    {X86_REG_DL,  REG_TYPE_RL,  X86_REG_EDX, "dl"},
    {X86_REG_DS,  REG_TYPE_NS,  X86_REG_DS,  NULL},
    {X86_REG_DX,  REG_TYPE_RX,  X86_REG_DX,  "dx"},
    {X86_REG_EAX, REG_TYPE_ERX, X86_REG_EAX, "eax"},
    {X86_REG_EBP, REG_TYPE_ERX, X86_REG_EBP, "ebp"},
    {X86_REG_EBX, REG_TYPE_ERX, X86_REG_EBX, "ebx"},
    {X86_REG_ECX, REG_TYPE_ERX, X86_REG_ECX, "ecx"},
    {X86_REG_EDI, REG_TYPE_ERX, X86_REG_EDI, "edi"},
    {X86_REG_EDX, REG_TYPE_ERX, X86_REG_EDX, "edx"},
    {X86_REG_EFLAGS, REG_TYPE_NS, X86_REG_EFLAGS, NULL},
    {X86_REG_EIP, REG_TYPE_ERX, X86_REG_EIP, "eip"},
    {X86_REG_EIZ, REG_TYPE_NS,  X86_REG_EIZ, NULL},
    {X86_REG_ES,  REG_TYPE_RX,  X86_REG_ESI, "es"},
    {X86_REG_ESI, REG_TYPE_ERX, X86_REG_ESI, "esi"},
    {X86_REG_ESP, REG_TYPE_ERX, X86_REG_ESP, "esp"},
    {X86_REG_FPSW, REG_TYPE_NS, X86_REG_FPSW, NULL},
    {X86_REG_FS,  REG_TYPE_NS,  X86_REG_FS,  NULL},
    {X86_REG_GS,  REG_TYPE_NS,  X86_REG_GS,  NULL},
    {X86_REG_IP,  REG_TYPE_RX,  X86_REG_EIP, "ip"}
    {-1, -1, NULL}
};

#define REG_STRINGS_NUM_ENTRIES (sizeof(reg_strings) / sizeof(index_string))


struct boper * asx86_get_register (struct list * binslist, x86_reg reg) {
    unsigned int index = 0xffff;

    /* If it looks like we don't have this register, double-check */
    if (    (reg > REG_STRINGS_NUM_ENTRIES)
         || (reg_strings[reg].index != reg)) {
        unsigned int i;
        for (i = 0; reg_strings[i].index != -1; i++) {
            if (reg_strings[i].index == reg) {
                index = i;
                break;
            }
        }
        /* Nope, don't have it */
        if (index == 0xffff)
            return NULL;
    }
    else
        /* This register is a perfect match in our lookup table */
        index = reg;

    const struct index_string * is = &(reg_strings[index]);
    const struct index_string * fis = &(reg_strings[is->full_index]);

    switch (is->type) {
    /* register is not supported */
    case REG_TYPE_NS :
        return NULL;
    /* rl type, like al */
    case REG_TYPE_RL :
        list_append_(binslist, bins_trun_(boper_variable(8, is->string),
                                          boper_variable(32, fis->string)));
        return boper_variable(8, is->string);
    /* rh type, like ah */
    case REG_TYPE_RH :
        list_append_(binslist, bins_shr_(boper_variable(32, "grtmp"),
                                         boper_variable(32, fis->string),
                                         boper_constant(32, 8)));
        list_append_(binslist, bins_trun_(boper_variable(8, is->string),
                                          boper_variable(32, "grtmp")));
        return boper_variable(8, is->string);
    /* rx type, like ax */
    case REG_TYPE_RX :
        list_append_(binslist, bins_trun_(boper_variable(16, is->string),
                                          boper_variable(32, fis->string)));
        return boper_variable(16, is->string);
    /* erx type, like eax */
    case REG_TYPE_ERX :
        return boper_variable(32, is->string);
    }
    return NULL;
}


int asx86_set_register (struct list * binslist,
                        x86_reg reg,
                        const struct boper * rhs) {
    unsigned int index = 0xffff;

    /* If it looks like we don't have this register, double-check */
    if (    (reg > REG_STRINGS_NUM_ENTRIES)
         || (reg_strings[reg].index != reg)) {
        unsigned int i;
        for (i = 0; reg_strings[i].index != -1; i++) {
            if (reg_strings[i].index == reg) {
                index = i;
                break;
            }
        }
        /* Nope, don't have it */
        if (index == 0xffff)
            return NULL;
    }
    else
        /* This register is a perfect match in our lookup table */
        index = reg;

    const struct index_string * is = &(reg_strings[index]);
    const struct index_string * fis = &(reg_strings[is->full_index]);

    switch (is->type) {
    case REG_TYPE_NS :
        return -1;
    case REG_TYPE_RL :
        /* rhs bit-width must match 8 */
        if (boper_bits(rhs) != 8)
            return -1;
        list_append_(binslist, bins_and_(boper_variable(32, fis->string),
                                         boper_variable(32, fis->string),
                                         boper_constant(32, 0xffffff00)));
        list_append_(binslist, bins_zext_(boper_variable(32, "srzext"),
                                          OCOPY(rhs)));
        list_append_(binslist, bins_or_(boper_variable(32, fis->string),
                                        boper_variable(32, fis->string),
                                        boper_variable(32, "srzext")));
        return 0;
    case REG_TYPE_RH :
        /* rhs bit-width must match 8 */
        if (boper_bits(rhs) != 8)
            return -1;
        list_append_(binslist, bins_and_(boper_variable(32, fis->string),
                                         boper_variable(32, fis->string),
                                         boper_constant(32, 0xffff00ff)));
        list_append_(binslist, bins_zext_(boper_variable(32, "srzext"),
                                          OCOPY(rhs)));
        list_append_(binslist, bins_shl_(boper_variable(32, "srzext"),
                                         boper_variable(32, "srzext"),
                                         boper_constant(32, 8)));
        list_append_(binslist, bins_or_(boper_variable(32, fis->string),
                                        boper_variable(32, fis->string),
                                        boper_variable(32, "srzext")));
        return 0;
    case REG_TYPE_RX :
        /* rhs bit-width must match 16 */
        if (boper_bits(rhs) != 16)
            return -1;
        list_append_(binslist, bins_and_(boper_variable(32, fis->string),
                                         boper_variable(32, fis->string),
                                         boper_constant(32, 0xffff0000)));
        list_append_(binslist, bins_zext_(boper_variable(32, "srzext"),
                                          OCOPY(rhs)));
        list_append_(binslist, bins_or_(boper_variable(32, "srzext"),
                                        boper_variable(32, "srzext"),
                                        boper_variable(32, fis->string)));
        return 0;
    case REG_TYPE_ERX :
        /* rhs bit-width must match 32 */
        if (boper_bits(rhs) != 32)
            return -1;
        list_append_(binslist, bins_or_(boper_variable(32, fis->string),
                                        OCOPY(rhs),
                                        OCOPY(rhs)));
        return 0;
    }
}


struct boper * asx86_operand (struct list * binslist, const cs_x86_op * op) {
    switch (op->type) {
    case X86_OP_REG :
        return asx86_get_register(binslist, op->reg);
    case X86_OP_IMM :
        return boper_constant(op->, op->imm);
    case X86_OP_MEM : {

    }
    case X86_OP_FP :
    default :
        return NULL;
    }
}


int asx86_store_le32_ (struct list * list,
                       struct boper * address,
                       struct boper * value) {
    list_append_(list, bins_trun_(boper_variable(8, "t8"), OCOPY(value)));
    list_append_(list, bins_store_(OCOPY(address), boper_variable(8, "t8")));

    list_append_(list, bins_shr_(boper_variable(32, "t32"),
                                 OCOPY(value),
                                 boper_constant(32, 8)));
    list_append_(list, bins_trun_(boper_variable(8, "t8"),
                                  boper_variable(32, "t32")));
    list_append_(list, bins_add_(boper_variable(32, "addr"),
                                 OCOPY(address),
                                 boper_constant(32, 1)));
    list_append_(list, bins_store_(boper_variable(32, "addr"),
                                   boper_variable(8, "t8")));

    list_append_(list, bins_shr_(boper_variable(32, "t32"),
                                 OCOPY(value),
                                 boper_constant(32, 16)));
    list_append_(list, bins_trun_(boper_variable(8, "t8"),
                                  boper_variable(32, "t32")));
    list_append_(list, bins_add_(boper_variable(32, "addr"),
                                 OCOPY(address),
                                 boper_constant(32, 2)));
    list_append_(list, bins_store_(boper_variable(32, "addr"),
                                   boper_variable(8, "t8")));

    list_append_(list, bins_shr_(boper_variable(32, "t32"),
                                 value,
                                 boper_constant(32, 24)));
    list_append_(list, bins_trun_(boper_variable(8, "t8"),
                                  boper_variable(32, "t32")));
    list_append_(list, bins_add_(boper_variable(32, "addr"),
                                 address,
                                 boper_constant(32, 3)));
    list_append_(list, bins_store_(boper_variable(32, "addr"),
                                   boper_variable(8, "t8")));
    return 0;
}


int asx86_store_le16_ (struct list * list,
                       struct boper * address,
                       struct boper * value) {
    list_append_(list, bins_trun_(boper_variable(8, "t8"), OCOPY(value)));
    list_append_(list, bins_store_(OCOPY(address), boper_variable(8, "t8")));

    list_append_(list, bins_shr_(boper_variable(16, "t16"),
                                 value,
                                 boper_constant(16, 8)));
    list_append_(list, bins_trun_(boper_variable(8, "t8"),
                                  boper_variable(16, "t16")));
    list_append_(list, bins_add_(boper_variable(32, "addr"),
                                 address,
                                 boper_constant(32, 1)));
    list_append_(list, bins_store(boper_variable(32, "addr"),
                                  boper_variable(8, "t8")));

    return 0;
}





struct list * asx86_translate_ins (const void * buf, size_t size) {
    const uint8_t * u8 = (const uint8_t *) buf;

    switch (u8[0]) {
    /* add al, imm8 */
    case 04 : {
        struct boper * al = asx86_reg_full_to_8(list, REG_EAX);
        list_append_(list, bins_add_(OCOPY(al),
                                     OCOPY(al),
                                     boper_constant(8, u8[1])));
        asx86_set_8bit_reg(list, REG_EAX, al);
        ODEL(al);
        break;
    }
    /* add ax, imm16 */
    case 05 : {
        struct boper * ax = asx86_reg_full_to_16(list, REG_EAX);
        list_append_(list, bins_add_(OCOPY(ax),
                                     OCOPY(ax),
                            boper_constant(16, asx86_buf_to_uint16(&(u8[1])))));
        asx86_set_16bit_reg(list, REG_EAX, ax);
        ODEL(ax);
    }
    /* add eax, imm32 */
    case 06 : {
        struct boper * eax = boper_variable(32, reg_strings[REG_EAX].s32);
        list_append_(list, bins_add_(OCOPY(eax),
                                     OCOPY(eax),
                            boper_constant(32, asx86_buf_to_uint32(&(u8[1])))));
        ODEL(eax);
    }
    case 08 : {
        switch ((u8[1] >> 3) & 0x7) {
        /* add r/m8, imm8 */
        case 0 :
            /* TODO */
        }
    }
}


struct list * asx86_translate_block (const void * buf, size_t size) {
    return asx86_translate_ins(buf, size);
}
