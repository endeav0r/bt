#include "amd64.h"

#include "container/memmap.h"


const struct arch_target arch_target_amd64 = {
    amd64_assemble,
    amd64_execute
};

/*
Convention:
When calling into hand-assembled code:
1) rbp points to the variable_space buffer
2) We only treat eax, ebx, ecx, edx, esi, edi as GPRs. Prefer eax/ebx/ecx/edx.
3) Never use esi or edi when operand size <= 8 
*/

#define REG_RAX 0x0
#define REG_RCX 0x1
#define REG_RDX 0x2
#define REG_RBX 0x3
#define REG_RSP 0x4
#define REG_RBP 0x5
#define REG_RSI 0x6
#define REG_RDI 0x7


int rm_off32_r (struct byte_buf * bb,
                unsigned int r,
                unsigned int rm,
                uint32_t off32) {
    byte_buf_append(bb, 0x80 | (r << 3) | rm);
    byte_buf_append_le32(bb, off32);
    return 0;
}

struct op_byte {
    uint8_t op8;
    uint8_t op32;
};

enum {
    JCC_JA,
    JCC_JAE,
    JCC_JB,
    JCC_JBE,
    JCC_JE,
    JCC_JG,
    JCC_JGE,
    JCC_JL,
    JCC_JLE,
};

struct op_byte jcc_op_bytes [] = {
    {0x77, 0x87}, // JA
    {0x73, 0x83}, // JAE
    {0x72, 0x82}, // JB
    {0x76, 0x86}, // JBE
    {0x74, 0x84}, // JE
    {0x7f, 0x8f}, // JG
    {0x7d, 0x8d}, // JGE
    {0x7c, 0x8c}, // JL
    {0x7e, 0x8e}, // JLE
};

enum {
    OP_ADD_RM_R,
    OP_AND_RM_R,
    OP_OR_RM_R,
    OP_SUB_RM_R,
    OP_XOR_RM_R,
    OP_MOV_RM_R_RM_R,
    OP_MOV_RM_R_R_RM,
};

struct op_byte op_rm_r_bytes [] = {
    {0x00, 0x01}, // add
    {0x20, 0x21}, // and
    {0x08, 0x09}, // or
    {0x28, 0x29}, // sub
    {0x30, 0x31}, // xor
    {0x88, 0x89}, // mov rm r
    {0x8a, 0x8b}  // mov r rm
};

int op_rm_r (struct byte_buf * bb,
             unsigned int op,
             unsigned int rm,
             uint32_t off32,
             unsigned int r,
             unsigned int bits) {
    switch (bits) {
    case 1 :
        // and [rm+off32] and r with 1
        and_rm_imm(bb, rm, off32, 1, 8);
        push_r64(bb, r);
        and_r_imm(bb, r, 1, 8);
        // op as an 8-bit op
        op_rm_r(bb, op, rm, off32, r, 8);
        // and result with 1
        and_rm_imm(bb, rm, off32, 1, 8);
        pop_r64(bb, r);
        return 0;
    case 8 :
        byte_buf_append(bb, op_rm_r_bytes[op].op8);
        return rm_off32_r(bb, r, rm, off32);
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, op_rm_r_bytes[op].op32);
        return rm_off32_r(bb, r, rm, off32);
    case 32 :
        byte_buf_append(bb, op_rm_r_bytes[op].op32);
        return rm_off32_r(bb, r, rm, off32);
    case 64 :
        byte_buf_append(bb, 0x48);
        byte_buf_append(bb, op_rm_r_bytes[op].op32);
        return rm_off32_r(bb, r, rm, off32);
    }
    return -1;
}

enum {
    OP_AND_RM_IMM
};

struct op_rm_imm_byte {
    unsigned int op8;
    unsigned int op32;
    unsigned int op_rm_r;
};

struct op_rm_imm_byte op_rm_imm_bytes [] = {
    {0x80, 0x81, OP_AND_RM_R}
};

int op_rm_imm (struct byte_buf * bb,
               unsigned int op,
               unsigned int rm,
               uint32_t off32,
               uint64_t imm,
               unsigned int bits) {
    switch (bits) {
    case 1 :
        and_rm_imm(bb, rm, off32, 1, 8);
        byte_buf_append(bb, op_rm_imm_bytes[op].op8);
        byte_buf_append(bb, 0xa0 | rm);
        byte_buf_append_le32(bb, off32);
        byte_buf_append(bb, imm & 1);
        and_rm_imm(bb, rm, off32, 1, 8);
        return 0;
    case 8 :
        byte_buf_append(bb, op_rm_imm_bytes[op].op8);
        byte_buf_append(bb, 0xa0 | rm);
        byte_buf_append_le32(bb, off32);
        byte_buf_append(bb, imm);
        return 0;
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, op_rm_imm_bytes[op].op32);
        byte_buf_append(bb, 0xa0 | rm);
        byte_buf_append_le32(bb, off32);
        byte_buf_append_le16(bb, imm);
        return 0;
    case 32 :
        byte_buf_append(bb, op_rm_imm_bytes[op].op32);
        byte_buf_append(bb, 0xa0 | rm);
        byte_buf_append_le32(bb, off32);
        byte_buf_append_le32(bb, imm);
        return 0;
    case 64 : {
        unsigned int rhs = REG_RAX;
        if (rm == REG_RAX)
            rhs = REG_RBX;
        push_r64(bb, rhs);
        mov_r_imm(bb, rhs, imm, 64);
        op_rm_r(bb, op_rm_imm_bytes[op].op_rm_r, rm, off32, rhs, 64);
        pop_r64(bb, rhs);
        return 0;
    }
    }
    return -1;
}


enum {
    OP_ADD_R_R,
    OP_AND_R_R,
    OP_CMP_R_R,
    OP_SUB_R_R,
};

struct op_byte op_r_r_bytes [] = {
    {0x00, 0x01},
    {0x20, 0x21},
    {0x38, 0x39},
    {0x28, 0x29}
};

int op_r_r (struct byte_buf * bb,
            unsigned int op,
            unsigned int dst,
            unsigned int rhs,
            unsigned int bits) {
    switch (bits) {
    case 1 :
        push_r64(bb, rhs);
        and_r_imm(bb, dst, 1, 1);
        and_r_imm(bb, rhs, 1, 1);
        op_r_r(bb, op, dst, rhs, 8);
        and_r_imm(bb, dst, 1, 1);
        pop_r64(bb, rhs);
        return 0;
    case 8 :
        byte_buf_append(bb, op_r_r_bytes[op].op8);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | dst);
        return 0;
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, op_r_r_bytes[op].op32);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | dst);
        return 0;
    case 32 :
        byte_buf_append(bb, op_r_r_bytes[op].op32);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | dst);
        return 0;
    case 64 :
        byte_buf_append(bb, 0x48);
        byte_buf_append(bb, op_r_r_bytes[op].op32);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | dst);
        return 0;
    }

    return -1;
}


enum {
    OP_ADD_R_IMM,
    OP_AND_R_IMM,
    OP_CMP_R_IMM,
    OP_SUB_R_IMM
};

struct op_r_imm_byte {
    unsigned int op8;
    unsigned int op32;
    unsigned char operand_byte;
    unsigned int op_r_r;
};


struct op_r_imm_byte op_r_imm_bytes [] = {
    {0x80, 0x81, 0xc0, OP_ADD_R_R},
    {0x80, 0x81, 0xe0, OP_AND_R_R},
    {0x80, 0x81, 0xf8, OP_CMP_R_R},
    {0x80, 0x81, 0xe8, OP_SUB_R_R}
};


int op_r_imm (struct byte_buf * bb,
              unsigned int op,
              unsigned int dst,
              uint64_t imm,
              unsigned int bits) {
    switch (bits) {
    case 1 :
        and_r_imm(bb, dst, 1, 8);
        op_r_imm(bb, op, dst, imm & 1, 8);
        and_r_imm(bb, dst, 1, 8);
        return 0;
    case 8 :
        byte_buf_append(bb, op_r_imm_bytes[op].op8);
        byte_buf_append(bb, op_r_imm_bytes[op].operand_byte | dst);
        byte_buf_append(bb, imm);
        return 0;
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, op_r_imm_bytes[op].op32);
        byte_buf_append(bb, op_r_imm_bytes[op].operand_byte | dst);
        byte_buf_append_le16(bb, imm);
        return 0;
    case 32 :
        byte_buf_append(bb, op_r_imm_bytes[op].op32);
        byte_buf_append(bb, op_r_imm_bytes[op].operand_byte | dst);
        byte_buf_append_le32(bb, imm);
        return 0;
    case 64 : {
        int rhs = REG_RAX;
        if (dst == REG_RAX)
            rhs = REG_RCX;
        push_r64(bb, rhs);
        mov_r_imm(bb, rhs, imm, 64);
        op_r_r(bb, op_r_imm_bytes[op].op_r_r, dst, rhs, 64);
        pop_r64(bb, rhs);
        return 0;
    }
    }

    return -1;
}


int add_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_ADD_RM_R, rm, off32, r, bits);
}


int add_r_imm (struct byte_buf * bb,
               unsigned int dst,
               uint64_t imm,
               unsigned int bits) {
    return op_r_imm(bb, OP_ADD_R_IMM, dst, imm, bits);
}


int add_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits) {
    return op_r_r(bb, OP_ADD_R_R, dst, rhs, bits);
}


int and_r_imm (struct byte_buf * bb,
               unsigned int dst,
               uint64_t imm,
               unsigned int bits) {
    return op_r_imm(bb, OP_AND_R_IMM, dst, imm, bits);
}


int and_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits) {
    return op_r_r(bb, OP_AND_R_R, dst, rhs, bits);
}


int and_rm_imm (struct byte_buf * bb,
                unsigned int rm,
                uint32_t off32,
                uint64_t imm,
                unsigned int bits) {
    return op_rm_imm(bb, OP_AND_RM_IMM, rm, off32, imm, bits);
}


int and_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_AND_RM_R, rm, off32, r, bits);
}


int call_r (struct byte_buf * bb, unsigned int r) {
    byte_buf_append(bb, 0xff);
    byte_buf_append(bb, 0xd0 | r);
    return 0;
}


int cmp_r_imm (struct byte_buf * bb,
               unsigned int r,
               uint64_t imm,
               unsigned int bits) {
    return op_r_imm(bb, OP_CMP_R_IMM, r, imm, bits);
}


int cmp_r_r (struct byte_buf * bb,
             unsigned int lhs,
             unsigned int rhs,
             unsigned int bits) {
    return op_r_r(bb, OP_CMP_R_R, lhs, rhs, bits);
}


int div_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs) {
    // save
    if (lhs != REG_RAX)
        push_r64(bb, REG_RAX);
    if (lhs != REG_RDX)
        push_r64(bb, REG_RDX);

    // prepare
    mov_r_r(bb, REG_RAX, lhs, 64);

    // execute
    byte_buf_append(bb, 0x48);
    byte_buf_append(bb, 0xf7);
    byte_buf_append(bb, 0xf0 | rhs);

    // set result
    mov_r_r(bb, lhs, REG_RAX, 64);

    if (lhs != REG_RDX)
        pop_r64(bb, REG_RDX);
    else if (lhs != REG_RAX)
        pop_r64(bb, REG_RAX);

    return 0;
}


int jcc (struct byte_buf * bb, unsigned int condition, int offset) {
    int abs_offset = offset;
    if (abs_offset < 0)
        abs_offset *= -1;

    if (abs_offset < 0x78) {
        byte_buf_append(bb, jcc_op_bytes[condition].op8);
        byte_buf_append(bb, offset - 2);
        return 0;
    }
    else if (abs_offset < 0x7ff0) {
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, 0x0f);
        byte_buf_append(bb, jcc_op_bytes[condition].op32);
        byte_buf_append(bb, offset - 5);
        return 0;
    }
    else if (abs_offset < 0x7ffffff0) {
        byte_buf_append(bb, 0x0f);
        byte_buf_append(bb, jcc_op_bytes[condition].op32);
        byte_buf_append_le32(bb, offset - 6);
        return 0;
    }
    return -1;
}


int jmp (struct byte_buf * bb, int offset) {
    int abs_offset = offset;
    if (abs_offset < 0)
        abs_offset *= -1;

    if (abs_offset < 0x78) {
        byte_buf_append(bb, 0xeb);
        byte_buf_append(bb, offset - 2);
        return 0;
    }
    else if (abs_offset < 0x7ff0) {
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, 0xe9);
        byte_buf_append_le16(bb, offset - 4);
        return 0;
    }
    else if (abs_offset < 0x7ffffff0) {
        byte_buf_append(bb, 0x0f);
        byte_buf_append(bb, 0xe9);
        byte_buf_append_le32(bb, offset - 6);
        return 0;
    }
    return -1;
}


int mod_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs) {
    // save
    if (lhs != REG_RAX)
        push_r64(bb, REG_RAX);
    if (lhs != REG_RDX)
        push_r64(bb, REG_RDX);

    // prepare
    mov_r_r(bb, REG_RAX, lhs, 64);

    // execute
    byte_buf_append(bb, 0x48);
    byte_buf_append(bb, 0xf7);
    byte_buf_append(bb, 0xf0 | rhs);

    // set result
    mov_r_r(bb, lhs, REG_RDX, 64);

    if (lhs != REG_RDX)
        pop_r64(bb, REG_RDX);
    else if (lhs != REG_RAX)
        pop_r64(bb, REG_RAX);

    return 0;
}


int mov_r_imm (struct byte_buf * bb,
               unsigned int r,
               uint64_t imm,
               unsigned int bits) {
    switch (bits) {
    case 8 :
        byte_buf_append(bb, 0xb0 | r);
        byte_buf_append(bb, imm);
        return 0;
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, 0xb8 | r);
        byte_buf_append_le16(bb, imm);
        return 0;
    case 32 :
        byte_buf_append(bb, 0xb8 | r);
        byte_buf_append_le32(bb, imm);
        return 0;
    case 64 :
        byte_buf_append(bb, 0x48);
        byte_buf_append(bb, 0xb8 | r);
        byte_buf_append_le64(bb, imm);
        return 0;
    }
    return -1;
}


int mov_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits) {
    switch (bits) {
    case 8 :
        byte_buf_append(bb, 0x88);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | dst);
        return 0;
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, 0x89);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | dst);
        return 0;
    case 32 :
        byte_buf_append(bb, 0x89);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | dst);
        return 0;
    case 64 :
        byte_buf_append(bb, 0x48);
        byte_buf_append(bb, 0x89);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | dst);
        return 0;
    }
    return -1;
}



int mov_r_rm (struct byte_buf * bb,
              unsigned int r,
              unsigned int rm,
              uint32_t off32,
              unsigned int bits) {
    return op_rm_r(bb, OP_MOV_RM_R_R_RM, rm, off32, r, bits);
}


int mov_rm_imm (struct byte_buf * bb,
                unsigned int rm,
                uint32_t off32,
                uint64_t imm,
                unsigned int bits) {
    switch (bits) {
    case 8 :
        byte_buf_append(bb, 0xc6);
        byte_buf_append(bb, 0x80 | rm);
        byte_buf_append_le32(bb, off32);
        byte_buf_append_le32(bb, imm);
        return 0;
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, 0xc7);
        byte_buf_append(bb, 0x80 | rm);
        byte_buf_append_le32(bb, off32);
        byte_buf_append_le32(bb, imm);
        return 0;
    case 32 :
        byte_buf_append(bb, 0xc7);
        byte_buf_append(bb, 0x80 | rm);
        byte_buf_append_le32(bb, off32);
        byte_buf_append_le32(bb, imm);
        return 0;
    case 64 : {
        unsigned int rhs = REG_RAX;
        if (rm == REG_RAX)
            rhs = REG_RCX;
        push_r64(bb, rhs);
        mov_r_imm(bb, rhs, imm, 64);
        mov_rm_r(bb, rm, off32, rhs, 64);
        pop_r64(bb, rhs);
        return 0;
    }
    }
    return -1;
}


int mov_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_MOV_RM_R_RM_R, rm, off32, r, bits);
}


int mov_r64_r64 (struct byte_buf * bb, unsigned int dst, unsigned int rhs) {
    byte_buf_append(bb, 0x48);
    byte_buf_append(bb, 0x89);
    byte_buf_append(bb, 0xc0 | (dst << 3) | rhs);
    return 0;
}


int movsx_r_r (struct byte_buf * bb,
               unsigned int dst,
               unsigned int dst_bits,
               unsigned int src,
               unsigned int src_bits) {
    if ((dst_bits == 16) && (src_bits == 8)) {
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, 0x0f);
        byte_buf_append(bb, 0xbe);
        byte_buf_append(bb, 0xc0 | (dst << 3) | src);
        return 0;
    }
    else if (dst_bits == 32) {
        if (src_bits == 8) {
            byte_buf_append(bb, 0x0f);
            byte_buf_append(bb, 0xbe);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
        else if (src_bits == 16) {
            byte_buf_append(bb, 0x0f);
            byte_buf_append(bb, 0xbf);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
    }
    else if (dst_bits == 64) {
        if (src_bits == 8) {
            byte_buf_append(bb, 0x48);
            byte_buf_append(bb, 0x0f);
            byte_buf_append(bb, 0xbe);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
        if (src_bits == 16) {
            byte_buf_append(bb, 0x48);
            byte_buf_append(bb, 0x0f);
            byte_buf_append(bb, 0xbf);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
        if (src_bits == 32) {
            byte_buf_append(bb, 0x48);
            byte_buf_append(bb, 0x63);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
    }
    return -1;
}


int movzx_r_r (struct byte_buf * bb,
               unsigned int dst,
               unsigned int dst_bits,
               unsigned int src,
               unsigned int src_bits) {
    if ((dst_bits == 16) && (src_bits == 8)) {
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, 0x0f);
        byte_buf_append(bb, 0xb6);
        byte_buf_append(bb, 0xc0 | (dst << 3) | src);
        return 0;
    }
    else if (dst_bits == 32) {
        if (src_bits == 8) {
            byte_buf_append(bb, 0x0f);
            byte_buf_append(bb, 0xb6);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
        else if (src_bits == 16) {
            byte_buf_append(bb, 0x0f);
            byte_buf_append(bb, 0xb6);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
    }
    else if (dst_bits == 64) {
        if (src_bits == 8) {
            byte_buf_append(bb, 0x48);
            byte_buf_append(bb, 0x0f);
            byte_buf_append(bb, 0xb6);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
        if (src_bits == 16) {
            byte_buf_append(bb, 0x48);
            byte_buf_append(bb, 0x0f);
            byte_buf_append(bb, 0xb6);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
        if (src_bits == 32) {
            mov_r_r(bb, dst, src, 32);
            return 0;
        }
    }
    return -1;
}


/*************************************
* mul reg, reg
*************************************/
// inefficient, but should be functionally correct
int mul_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs) {
    /* What are we destroying today?
      * rax (lhs and dst)
      * rdx high bits of result
    */

    // save
    if (lhs != REG_RAX)
        push_r64(bb, REG_RAX);
    else if (lhs != REG_RDX)
        push_r64(bb, REG_RDX);

    // prepare
    mov_r_r(bb, REG_RAX, lhs, 64);

    // execute
    byte_buf_append(bb, 0x48);
    byte_buf_append(bb, 0xf7);
    byte_buf_append(bb, 0xe0 | rhs);

    // set result
    mov_r_r(bb, lhs, REG_RAX, 64);

    // restore
    if (lhs != REG_RDX)
        pop_r64(bb, REG_RDX);
    else if (lhs != REG_RAX)
        pop_r64(bb, REG_RAX);

    return 0;
}



/*************************************
* or [REG+OFF32], REG
*************************************/
int or_rm_r (struct byte_buf * bb,
             unsigned int rm,
             uint32_t off32,
             unsigned int r,
             unsigned int bits) {
    return op_rm_r(bb, OP_OR_RM_R, rm, off32, r, bits);
}


int pop_r64 (struct byte_buf * bb, unsigned int reg) {
    byte_buf_append(bb, 0x58 | reg);
    return 0;
}


int push_r64 (struct byte_buf * bb, unsigned int reg) {
    byte_buf_append(bb, 0x50 | reg);
    return 0;
}


int ret (struct byte_buf * bb) {
    byte_buf_append(bb, 0xc3);
    return 0;
}


int shl_r64_r64 (struct byte_buf * bb,
                 unsigned int lhs,
                 unsigned int rhs) {
    /*
        We can only shift left by an 8-bit value, however btins requires all 
        operands to be of the same size, and our shift size can be any valid
        value up to 64-bits. If we truncate a 64-bit value to 8-bits, and the
        shift size is >= 256, we could shift the wrong number of bits.
        Therefor, if the shift value is greater than bits, we set the lhs to
        0.
    */

    // generate bytes to set lhs to zero
    struct byte_buf * zero_buf = byte_buf_create();
    mov_r_imm(zero_buf, lhs, 0, 64);

    // now generate bytes to shl
    struct byte_buf * shift_buf = byte_buf_create();
    if (lhs != REG_RCX)
        push_r64(shift_buf, REG_RCX);
    mov_r_r(shift_buf, REG_RCX, rhs, 8);
    byte_buf_append(shift_buf, 0x48);
    byte_buf_append(shift_buf, 0xd3);
    byte_buf_append(shift_buf, 0xe0 | lhs);
    if (lhs != REG_RCX)
        pop_r64(shift_buf, REG_RCX);
    // jump over the zero buf
    jmp(shift_buf, byte_buf_length(zero_buf));

    // check if rhs is greater than 64
    cmp_r_imm(bb, rhs, 64, 64);
    // if above or equal, jump over shift_buf
    jcc(bb, JCC_JAE, byte_buf_length(shift_buf));
    byte_buf_append_byte_buf(bb, shift_buf);
    byte_buf_append_byte_buf(bb, zero_buf);

    byte_buf_delete(shift_buf);
    byte_buf_delete(zero_buf);

    return 0;
}


int shr_r64_r64 (struct byte_buf * bb,
                 unsigned int lhs,
                 unsigned int rhs) {
    /*
        SHR has same problems as shl
    */

    // generate bytes to set lhs to zero
    struct byte_buf * zero_buf = byte_buf_create();
    mov_r_imm(zero_buf, lhs, 0, 64);

    // now generate bytes to shl
    struct byte_buf * shift_buf = byte_buf_create();
    if (lhs != REG_RCX)
        push_r64(shift_buf, REG_RCX);
    mov_r_r(shift_buf, REG_RCX, rhs, 8);
    byte_buf_append(shift_buf, 0x48);
    byte_buf_append(shift_buf, 0xd3);
    byte_buf_append(shift_buf, 0xe8 | lhs);
    if (lhs != REG_RCX)
        pop_r64(shift_buf, REG_RCX);
    // jump over the zero buf
    jmp(shift_buf, byte_buf_length(zero_buf));

    // check if rhs is greater than 64
    cmp_r_imm(bb, rhs, 64, 64);
    // if above or equal, jump over shift_buf
    jcc(bb, JCC_JAE, byte_buf_length(shift_buf));
    byte_buf_append_byte_buf(bb, shift_buf);
    byte_buf_append_byte_buf(bb, zero_buf);

    byte_buf_delete(shift_buf);
    byte_buf_delete(zero_buf);

    return 0;
}


int sub_r_imm (struct byte_buf * bb,
             unsigned int dst,
             uint64_t imm,
             unsigned int bits) {
    return op_r_imm(bb, OP_SUB_R_IMM, dst, imm, bits);
}


int sub_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits) {
    return op_r_imm(bb, OP_SUB_R_R, dst, rhs, bits);
}


int sub_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_SUB_RM_R, rm, off32, r, bits);
}


int xor_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_XOR_RM_R, rm, off32, r, bits);
}


int amd64_load_r_boper (struct byte_buf * bb,
                        struct varstore * varstore,
                        unsigned int reg,
                        struct boper * boper) {
    if (boper_type(boper) == BOPER_CONSTANT)
        return mov_r_imm(bb, reg, boper_value(boper), boper_bits(boper));
    else {
        size_t offset = varstore_offset_create(varstore,
                                               boper_identifier(boper),
                                               boper_bits(boper));
        return mov_r_rm(bb, reg, REG_RBP, offset, boper_bits(boper));
    }
}


int amd64_store_boper_r (struct byte_buf * bb,
                         struct varstore * varstore,
                         struct boper * boper,
                         unsigned int reg) {
    size_t offset = varstore_offset_create(varstore,
                                           boper_identifier(boper),
                                           boper_bits(boper));
    return mov_rm_r(bb, REG_RBP, offset, reg, boper_bits(boper));
}


int amd64_store_boper_imm (struct byte_buf * bb,
                           struct varstore * varstore,
                           struct boper * boper,
                           uint64_t imm) {
    size_t offset = varstore_offset_create(varstore,
                                           boper_identifier(boper),
                                           boper_bits(boper));
    return mov_rm_imm(bb, REG_RBP, offset, imm, boper_bits(boper));
}


struct byte_buf * amd64_assemble (struct list * btins_list,
                                  struct varstore * varstore) {
    int error = 0;

    struct byte_buf * bb = byte_buf_create();
    struct list_it * it;
    for (it = list_it(btins_list); it != NULL; it = list_it_next(it)) {
        struct bins * bins = list_it_obj(it);

        switch (bins->op) {
        // arithmetic instructions that operate directly against rm
        case BOP_ADD :
        case BOP_SUB :
        case BOP_AND :
        case BOP_OR  :
        case BOP_XOR : {
            // if we need to move lhs into dst
            if (boper_cmp(bins->oper[0], bins->oper[1])) {
                if (boper_type(bins->oper[1]) == BOPER_CONSTANT)
                    amd64_store_boper_imm(bb,
                                          varstore,
                                          bins->oper[0],
                                          boper_value(bins->oper[1]));
                else {
                    amd64_load_r_boper(bb,
                                       varstore,
                                       REG_RAX,
                                       bins->oper[1]);
                    amd64_store_boper_r(bb,
                                        varstore,
                                        bins->oper[0],
                                        REG_RAX);
                }
            }
            // load rhs into register
            if (boper_type(bins->oper[2]) == BOPER_CONSTANT) {
                mov_r_imm(bb,
                          REG_RAX,
                          boper_value(bins->oper[2]),
                          boper_bits(bins->oper[2]));
            }
            else {
                amd64_load_r_boper(bb,
                                   varstore,
                                   REG_RAX,
                                   bins->oper[2]);
            }
            // get offset to dst
            size_t offset = varstore_offset_create(varstore,
                                                   boper_identifier(bins->oper[0]),
                                                   boper_bits(bins->oper[0]));
            switch (bins->op) {
            case BOP_ADD :
                add_rm_r(bb, REG_RBP, offset, REG_RAX, boper_bits(bins->oper[0]));
                break;
            case BOP_SUB :
                sub_rm_r(bb, REG_RBP, offset, REG_RAX, boper_bits(bins->oper[0]));
                break;
            case BOP_AND :
                and_rm_r(bb, REG_RBP, offset, REG_RAX, boper_bits(bins->oper[0]));
                break;
            case BOP_OR :
                or_rm_r(bb, REG_RBP, offset, REG_RAX, boper_bits(bins->oper[0]));
                break;
            case BOP_XOR :
                xor_rm_r(bb, REG_RBP, offset, REG_RAX, boper_bits(bins->oper[0]));
                break;
            }
            break;
        }
        // arithmetic instructions that operate against r64, r64
        case BOP_UMUL :
        case BOP_UDIV :
        case BOP_UMOD :
        case BOP_SHL :
        case BOP_SHR : {
            // load lhs and rhs into RAX and RBX
            amd64_load_r_boper(bb, varstore, REG_RAX, bins->oper[1]);
            amd64_load_r_boper(bb, varstore, REG_RBX, bins->oper[2]);
            // are these 64-bit operands?
            if (boper_bits(bins->oper[1]) != 64) {
                movzx_r_r(bb, REG_RAX, 64, REG_RAX, boper_bits(bins->oper[1]));
                movzx_r_r(bb, REG_RBX, 64, REG_RBX, boper_bits(bins->oper[2]));
            }
            // perform op
            switch (bins->op) {
            case BOP_UMUL :
                mul_r64_r64(bb, REG_RAX, REG_RBX);
                break;
            case BOP_UDIV :
                div_r64_r64(bb, REG_RAX, REG_RBX);
                break;
            case BOP_UMOD :
                mod_r64_r64(bb, REG_RAX, REG_RBX);
                break;
            case BOP_SHL :
                shl_r64_r64(bb, REG_RAX, REG_RBX);
                break;
            case BOP_SHR :
                shr_r64_r64(bb, REG_RAX, REG_RBX);
                break;
            }
            // store result
            amd64_store_boper_r(bb, varstore, bins->oper[0], REG_RAX);
            break;
        }
        // our conditionals
        case BOP_CMPEQ :
        case BOP_CMPLTU :
        case BOP_CMPLTS :
        case BOP_CMPLEU :
        case BOP_CMPLES : {

            /*
            * cmpop:
            *   cmp
            *   jcc al1
            * al0 :
            *   mov al, 0
            *   jmp result
            * al1 :
            *   mov al, 1
            * result :
            *   store result
            */
            // create byte buffers for settings result

            struct byte_buf * cmpop = byte_buf_create();
            struct byte_buf * al0 = byte_buf_create();
            struct byte_buf * al1 = byte_buf_create();

            if (boper_type(bins->oper[1]) == BOPER_CONSTANT)
                mov_r_imm(cmpop,
                          REG_RAX,
                          boper_value(bins->oper[1]),
                          boper_bits(bins->oper[1]));
            else
                amd64_load_r_boper(cmpop, varstore, REG_RAX, bins->oper[1]);

            if (boper_type(bins->oper[2]) == BOPER_CONSTANT)
                mov_r_imm(cmpop,
                          REG_RCX,
                          boper_value(bins->oper[2]),
                          boper_bits(bins->oper[2]));
            else
                amd64_load_r_boper(cmpop, varstore, REG_RCX, bins->oper[1]);

            cmp_r_r(cmpop, REG_RAX, REG_RCX, boper_bits(bins->oper[1]));

            mov_r_imm(al1, REG_RAX, 8, 1);

            mov_r_imm(al0, REG_RAX, 8, 0);
            jmp(al0, byte_buf_length(al1));

            switch (bins->op) {
            case BOP_CMPEQ :
                jcc(cmpop, JCC_JE, byte_buf_length(al0));
                break;
            case BOP_CMPLTU :
                jcc(cmpop, JCC_JB, byte_buf_length(al0));
                break;
            case BOP_CMPLTS :
                jcc(cmpop, JCC_JL, byte_buf_length(al0));
                break;
            case BOP_CMPLEU :
                jcc(cmpop, JCC_JBE, byte_buf_length(al0));
                break;
            case BOP_CMPLES :
                jcc(cmpop, JCC_JLE, byte_buf_length(al0));
                break;
            }

            byte_buf_append_byte_buf(bb, cmpop);
            byte_buf_append_byte_buf(bb, al0);
            byte_buf_append_byte_buf(bb, al1);

            amd64_store_boper_imm(bb, varstore, bins->oper[0], REG_RAX);

            ODEL(cmpop);
            ODEL(al0);
            ODEL(al1);
            break;
        }
        case BOP_SEXT :
            amd64_load_r_boper(bb, varstore, REG_RAX, bins->oper[1]);
            movsx_r_r(bb,
                      REG_RAX,
                      boper_bits(bins->oper[0]),
                      REG_RAX,
                      boper_bits(bins->oper[1]));
            amd64_store_boper_r(bb, varstore, bins->oper[0], REG_RAX);
            break;
        case BOP_ZEXT :
            amd64_load_r_boper(bb, varstore, REG_RAX, bins->oper[1]);
            movzx_r_r(bb,
                      REG_RAX,
                      boper_bits(bins->oper[0]),
                      REG_RAX,
                      boper_bits(bins->oper[1]));
            amd64_store_boper_r(bb, varstore, bins->oper[0], REG_RAX);
            break;
        case BOP_TRUN :
            amd64_load_r_boper(bb, varstore, REG_RAX, bins->oper[1]);
            amd64_store_boper_r(bb, varstore, bins->oper[0], REG_RAX);
            break;
        case BOP_LOAD : {
            /* set up call to mmap_get_u8 */
            size_t offset;
            if (varstore_offset(varstore, "__MMAP__", 64, &offset)) {
                error = -1;
                break;
            }
            // prepare call
            mov_r_rm(bb, REG_RDI, REG_RBP, offset, 64);
            amd64_load_r_boper(bb, varstore, REG_RSI, bins->oper[1]);
            // create scratch space
            sub_r_imm(bb, REG_RSP, 8, 64);
            mov_r_r(bb, REG_RDX, REG_RSP, 64);

            // execute call
            mov_r_imm(bb, REG_RAX, (uint64_t) memmap_get_u8, 64);
            call_r(bb, REG_RAX);

            // clean up scratch space

            // if failure, we set rax to 1 and return
            struct byte_buf * fail = byte_buf_create();
            add_r_imm(fail, REG_RSP, 8, 64);
            mov_r_imm(fail, REG_RAX, 1, 64);
            ret(fail);

            // if success, read byte off stack and set variable
            struct byte_buf * success = byte_buf_create();
            // mov al, [rsp+0x00000000] is not a valid instruction
            mov_r_r(success, REG_RAX, REG_RSP, 64);
            mov_r_rm(success, REG_RAX, REG_RAX, 0, 8);
            add_r_imm(fail, REG_RSP, 8, 64);
            amd64_store_boper_r(bb, varstore, bins->oper[0], REG_RAX);

            // compare result of our call and execution conditionally
            cmp_r_imm(bb, REG_RAX, 0, 64);
            jcc(bb, JCC_JE, byte_buf_length(fail));
            byte_buf_append_byte_buf(bb, fail);
            byte_buf_append_byte_buf(bb, success);

            ODEL(fail);
            ODEL(success);
            break;
        }
        case BOP_STORE : {
            // set up a call to mmap_set_u8
            size_t offset;
            if (varstore_offset(varstore, "__MMAP__", 64, &offset)) {
                error = -1;
                break;
            }

            // prepare call
            mov_r_rm(bb, REG_RDI, REG_RBP, offset, 64);
            amd64_load_r_boper(bb, varstore, REG_RSI, bins->oper[0]);
            // create scratch space
            sub_r_imm(bb, REG_RSP, 8, 64);
            // move value into byte pointed to by rsp
            mov_r_r(bb, REG_RDX, REG_RSP, 64);
            amd64_load_r_boper(bb, varstore, REG_RAX, bins->oper[1]);
            mov_rm_r(bb, REG_RDX, 0, REG_RCX, 8);
            // execute call
            mov_r_imm(bb, REG_RAX, (uint64_t) memmap_set_u8, 64);
            call_r(bb, REG_RAX);

            // if fail, set rax to 2 and return
            struct byte_buf * fail = byte_buf_create();
            add_r_imm(fail, REG_RSP, 8, 64);
            mov_r_imm(fail, REG_RAX, 2, 64);
            ret(fail);

            // compare result and execute fail condition if necessary
            cmp_r_imm(bb, REG_RAX, 0, 64);
            jcc(bb, JCC_JE, byte_buf_length(fail));

            byte_buf_append_byte_buf(bb, fail);

            ODEL(fail);
            break;
        }
        case BOP_HLT :
            // return 3
            mov_r_imm(bb, REG_RAX, 3, 64);
            ret(bb);
            break;
        }
    }
    if (error) {
        ODEL(bb);
        return NULL;
    }
    mov_r_imm(bb, REG_RAX, 0, 64);
    ret(bb);
    return bb;
}


unsigned int amd64_execute (const void * code,
                            struct varstore * varstore) {
    unsigned int result;
    void * data_buf = varstore_data_buf(varstore);
    
    asm(
        "push %%rbx;"
        "push %%rbp;"
        "mov %1, %%rbp;"
        "mov %2, %%rax;"

        // mac os x inline assemble is ridiculous, this is a workaround
        "push %%rax;"
        "call *(%%rsp);"
        "pop %%rbp;"

        "pop %%rbp;"
        "pop %%rbx;"
        "mov %%eax, %0;"
        : "=r" (result)
        : "r" (data_buf), "r" (code)
        : "rax", "rcx", "rdx", "rdi", "rsi"
    );
    /*
    asm(
        ".intel_syntax noprefix;"
        "push rbx;"
        "push rbp;"
        "mov rbp, %1;"
        "mov rax, %2;"
        "call rax;"
        "pop rbp;"
        "pop rbx;"
        "mov %0, rax;"
        ".att_syntax;"
        : "=r" (result)
        : "r" (data_buf), "r" (code)
        : "rax", "rcx", "rdx", "rdi", "rsi"
    );
    */
    return result;
}
