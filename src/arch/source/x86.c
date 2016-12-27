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
    uint32_t r |= buf[1] << 8;
    r |= buf[0];
    return r;
}


struct index_string {
    int index;
    const char * s32;
    const char * s16;
    const char * sh;
    const char * sl;
};


const struct index_string reg_strings[] = {
    {REG_EAX, "eax", "ax", "ah", "al"},
    {REG_ECX, "ecx", "cx", "ch", "cl"},
    {REG_EDX, "edx", "dx", "dh", "dl"},
    {REG_EBX, "ebx", "bx", "bh", "bl"},
    {REG_ESP, "esp", "sp", NULL, NULL},
    {REG_EBP, "ebp", "bp", NULL, NULL},
    {REG_ESI, "esi", "si", NULL, NULL},
    {REG_EDI, "edi", "di", NULL, NULL},
    {-1, NULL}
};


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


struct boper * asx86_reg_full_to_8 (struct list * list, unsigned int reg) {
    struct boper * r8 = boper_variable(8, reg_strings[reg].sl);
    struct boper * r32 = boper_variable(32, reg_strings[reg].s32);
    list_append_(list, bins_trun(r8, r32));
    ODEL(r32);
    return r8;
}


struct boper * asx86_reg_full_to_16 (struct list * list, unsigned int reg) {
    struct boper * r16 = boper_variable(16, reg_strings[reg].s16);
    struct boper * r32 = boper_variable(32, reg_strings[reg].s32);
    list_append_(list, bins_trun(r16, r32));
    ODEL(r32);
    return r16;
}


int asx86_set_8bit_reg (struct list * list,
                        unsigned int reg,
                        struct boper * value) {
    struct boper * fullreg = boper_variable(32, reg_strings[reg].s32);
    list_append_(list, bins_and_(OCOPY(fullreg),
                                 OCOPY(fullreg),
                                 boper_constant(32, 0xffffff00)));
    struct boper * tmp32 = boper_variable(32, "tmp32");
    list_append_(list, bins_zext_(tmp32, OCOPY(value)));
    list_append_(list, bins_or_(OCOPY(fullreg),
                                fullreg,
                                tmp32));
    return 0;
}


int asx86_set_16bit_reg (struct list * list,
                        unsigned int reg,
                        struct boper * value) {
    struct boper * fullreg = boper_variable(32, reg_strings[reg].s32);
    list_append_(list, bins_and_(OCOPY(fullreg),
                                 OCOPY(fullreg),
                                 boper_constant(32, 0xffff0000)));
    struct boper * tmp32 = boper_variable(32, "tmp32");
    list_append_(list, bins_zext_(tmp32, OCOPY(value)));
    list_append_(list, bins_or_(OCOPY(fullreg),
                                fullreg,
                                tmp32));
    return 0;
}


struct boper * asx86_sib (struct list * list,
                          const uint8_t * bytes,
                          size_t bytes_size) {

    uint8_t scale_bits = bytes[1] >> 6;
    uint8_t index_bits = (bytes[1] >> 3) & 0x7;
    uint8_t base_bits  = bytes[1] & 0x7;
    uint8_t mod_bits   = bytes[0] >> 6;

    struct boper * sib = NULL;

    /* calculate scaled index first */
    if (index_bits != 4) {
        switch (scale_bits) {
        case 0 :
            sib = boper_variable(32, reg_strings[index_bits].s32);
            break;
        case 1 : {
            sib = boper_variable(32, "sib");
            list_append_(list, bins_umul_(OCOPY(sib),
                                boper_variable(32, reg_strings[index_bits].s32),
                                boper_constant(32, 2)));
            break;
        }
        case 2 : {
            sib = boper_variable(32, "sib");
            list_append_(list, bins_umul_(OCOPY(sib),
                                boper_variable(32, reg_strings[index_bits].s32),
                                boper_constant(32, 4)));
            break;
        }
        case 3 : {
            sib = boper_variable(32, "sib");
            list_append_(list, bins_umul_(OCOPY(sib),
                                boper_variable(32, reg_strings[index_bits].s32),
                                boper_constant(32, 8)));
            break;
        }
        }
    }

    /* now let's get the base */
    /* when base is EBP and mod_bits is 0, we have no base */
    if ((base_bits != 5) && (mod_bits != 0)) {
        if (sib == NULL)
            sib = boper_variable(32, reg_strings[base_bits].s32);
        else {
            list_append_(list, bins_add_(OCOPY(sib),
                              OCOPY(sib),
                              boper_variable(32, reg_strings[base_bits].s32)));
        }
    }

    return sib;
}


/*
    MOD XX
    RM       ZZZ
    REG   YYY
    ------------
        XXYYYZZZ

    mod 00 -> [rm], reg
    mod 01 -> [rm+disp8], reg
    mod 10 -> [rm+disp32], reg
    mod 11 -> rm, reg

    In instances where rm is a memory reference, i.e. [rm], this indicates a
    SIB follows.
*/
struct boper * asx86_modrm_ea (struct list * list,
                               const uint8_t * bytes,
                               size_t bytes_size,
                               unsigned int bits) {

    uint8_t modrm_byte = bytes[0];
    uint8_t mod = (modrm_byte >> 6) & 0x3;
    uint8_t rm = modrm_byte & 0x7;

    struct boper * ea = NULL;

    switch (mod) {
    /* MOD = 00b */
    case 0x00 : {
        /* sib */
        if (rm == 0x4) {
            if (bytes_size < 2)
                return NULL;
            ea = asx86_sib(list, bytes, bytes_size);
        }
        /* displacement */
        else if (rm == 0x5) {
            if (bytes_size < 5)
                return NULL;
            ea = boper_constant(32, asx86_buf_to_uint32(&(bytes[1])));
        }
        else {
          ea = boper_variable(32, reg_strings[rm].s32);
        }
        break;
    }
    /* MOD = 01b */
    case 0x01 : {
        uint32_t disp = 0;
        /* sib */
        if (rm == 4) {
            if (bytes_size < 3)
                return NULL;
            disp = bytes[2];
            ea = asx86_sib(list, bytes, bytes_size);
        }
        /* normal mod r/m */
        else {
            if (bytes_size < 2)
                return NULL;
            disp = bytes[1];
        }

        /* sign extend our displacement */
        if (disp & 0x80)
            disp |= 0xffffff00;

        /* if ea is already set (ie sib) add disp to it */
        if (ea != NULL) {
            list_append_(list, bins_add_(OCOPY(ea),
                                         OCOPY(ea),
                                         boper_constant(32, disp)));
        }
        /* else we need to add the disp to the correct register */
        else {
            ea = boper_variable(32, "ea");
            list_append_(list, bins_add_(OCOPY(ea),
                                         boper_constant(32, disp),
                                      boper_variable(32, reg_strings[rm].s32)));
        }
        break;
    }
    case 0x02 : {
        uint32_t disp = 0;
        if (rm == 4) {
            if (bytes_size < 6)
                return NULL;
            disp = asx86_buf_to_uint32(&(bytes[2]));
            ea = asx86_sib(list, bytes, bytes_size);
        }
        else {
            if (bytes_size < 5)
                return NULL;
            disp = asx86_buf_to_uint32(&(bytes[1]));
        }

        if (ea != NULL) {
            list_append_(list, bins_add_(OCOPY(ea),
                                         OCOPY(ea),
                                         boper_constant(32, disp)));
        }
        else {
            ea = boper_variable(32, "ea");
            list_append_(list, bins_add_(OCOPY(ea),
                                         boper_constant(32, disp),
                                      boper_variable(32, reg_strings[rm].s32)));
        }
        break;
        }
    case 0x03 :
        switch (bits) {
        case 8 :
            ea = asx86_reg_full_to_8(list, rm);
            break;
        case 16 :
            ea = asx86_reg_full_to_16(list, rm);
            break;
        case 32 :
            ea = boper_variable(32, reg_strings[rm].s32);
            break;
        }
        break;
    }
    return ea;
}


struct boper * asx86_modrm_value (struct list * list,
                                  const uint8_t * bytes,
                                  size_t bytes_size,
                                  unsigned int bits) {
    switch (bits) {
    case 8 : return asx86_reg_full_to_8(list, (bytes[0] >> 3) & 0x7);
    case 16 : return asx86_reg_full_to_16(list, (bytes[0] >> 3) & 0x7);
    case 32 : return boper_variable(32, reg_strings[(bytes[0] >> 3) & 0x7]);
    }
    return NULL;
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
