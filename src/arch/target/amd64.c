#include "container/byte_buf.h"

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


int r_rm_off32 (struct byte_buf * bb,
                unsigned int r,
                unsigned int rm,
                uint32_t off32) {
    byte_buf_append(bb, 0x80 | (r << 3) | rm);
    byte_buf_append_le32(bb, off32);
    return 0;
}

struct op_byte = {
    uint8_t op8,
    uint8_t op32
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
    OP_RM_R_ADD,
    OP_RM_R_SUB,
    OP_RM_R_AND,
    OP_RM_R_OR,
    OP_RM_R_XOR,
    OP_RM_R_MOV_RM_R,
    OP_RM_R_MOV_R_RM,
};

struct op_byte op_rm_r_bytes [] = {
    {0x00, 0x01},
    {0x28, 0x29},
    {0x20, 0x21},
    {0x08, 0x09},
    {0x30, 0x31},
    {0x88, 0x89},
    {0x8a, 0x8b}
};

int op_rm_r (struct byte_buf * bb,
             unsigned int op,
             unsigned int rm,
             uint32_t off32,
             unsigned int r,
             unsigned int bits) {
    switch (bits) {
    case 8 :
        byte_buf_append(bb, op_rm_r_bytes[op].op8);
        return r_rm_off32(bb, r, rm, off32);
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, op_rm_r_bytes[op].op32);
        return r_rm_off32(bb, r, rm, off32);
    case 32 :
        byte_buf_append(bb, op_rm_r_bytes[op].op32);
        return r_rm_off32(bb, r, rm, off32);
    case 64 :
        byte_buf_append(bb, 0x48);
        byte_buf_append(bb, op_rm_r_bytes[op].op32);
        return r_rm_off32(bb, r, rm, off32);
    }
    return -1;
}


int add_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_RM_R_ADD, rm, off32, r, bits);
}


int and_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_RM_R_AND, rm, off32, r, bits);
}


int and_r8_imm8 (struct byte_buf * bb, unsigned int r8, uint8_t imm8) {
    switch (r8) {
    case REG_RAX :
    case REG_RCX :
    case REG_RDX :
    case REG_RBX :
        byte_buf_append(bb, 0x80);
        byte_buf_append(bb, 0xe0 & source_register);
        byte_buf_append(bb, imm8);
        return 0;
    }
    return -1;
}


int and_r16_imm16 (struct byte_buf * bb, unsigned int r16, uint16_t imm16) {
    byte_buf_append(bb, 0x66);
    byte_buf_append(bb, 0x81);
    byte_buf_append(bb, 0xe0 | r16);
    byte_buf_append_le16(bb, imm16);
    return 0;
}


int cmp_r_r (struct byte_buf * bb,
             unsigned int lhs,
             unsigned int rhs,
             unsigned int bits) {
    switch (bits) {
    case 8 :
        byte_buf_append(bb, 0x38);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | lhs);
        return 0;
    case 16 :
        byte_buf_append(bb, 0x66);
        byte_buf_append(bb, 0x39);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | lhs);
        return 0;
    case 32 :
        byte_buf_append(bb, 0x39);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | lhs);
        return 0;
    case 64 :
        byte_buf_append(bb, 0x48);
        byte_buf_append(bb, 0x39);
        byte_buf_append(bb, 0xc0 | (rhs << 3) | lhs);
        return 0;
    }
}


int cmp_r_imm (struct byte_buf * bb,
               unsigned int r,
               uint64_t imm,
               unsigned int bits) {
    switch (bits) {
    case 8 :
        byte_buf_append(bb, 0x80);
        byte_buf_append(bb, 0xf8 | r);
        byte_buf_append(bb, imm);
        return 0;
    case 16 :
        byte_buf_append(bb, 0x66)
        byte_buf_append(bb, 0x81);
        byte_buf_append(bb, 0xf8 | r);
        byte_buf_append_le16(bb, imm);
        return 0;
    case 32 :
        byte_buf_append(bb, 0x81);
        byte_buf_append(bb, 0xf8 | r);
        byte_buf_append_le32(bb, imm);
        return 0;
    case 64 : {
        unsigned int rhs = REG_RAX;
        if (r == REG_RAX)
            rhs = REG_RBX;
        push_r64(bb, rhs);
        mov_r_imm(bb, rhs, imm, 64);
        cmp_r_r(bb, r, rhs, 64);
        pop_r64(bb, rhs);
        return 0;
    }
    }
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
        if (offset < 0)
            byte_buf_append(bb, offset - 2);
        else
            byte_buf_append(bb, offset);
        return 0;
    }
    else if (abs_offset < 0x7ff0) {
        byte_buf_append(bb, 0x66)
        byte_buf_append(bb, 0x0f);
        byte_buf_append(bb, jcc_op_bytes[condition].op32);
        if (offset < 0)
            byte_buf_append_le16(bb, offset - 5);
        else
            byte_buf_append(bb, offset);
        return 0;
    }
    else if (abs_offset < 0x7ffffff0) {
        byte_buf_append(bb, 0x0f);
        byte_buf_append(bb, jcc_op_bytes[condition].op32);
        if (offset < 0)
            byte_buf_append_le32(bb, offset - 6);
        else
            byte_buf_append(bb, offset);
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
        if (offset < 0)
            byte_buf_append(bb, offset - 2);
        else
            byte_buf_append(bb, offset);
    }
    else if (abs_offset < 0x7ff0) {
        byte_buf_append(bb, 0x66)
        byte_buf_append(bb, 0xe9);
        if (offset < 0)
            byte_buf_append_le16(bb, offset - 4);
        else
            byte_buf_append(bb, offset);
        return 0;
    }
    else if (abs_offset < 0x7ffffff0) {
        byte_buf_append(bb, 0x0f);
        byte_buf_append(bb, 0xe9);
        if (offset < 0)
            byte_buf_append_le32(bb, offset - 6);
        else
            byte_buf_append(bb, offset);
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
    mov_r_r(bb, REG_RAX, lhs);

    // execute
    byte_buf_append(bb, 0x48);
    byte_buf_append(bb, 0xf7);
    byte_buf_append(bb, 0xf0 | rhs);

    // set result
    mov_r_r(bb, lhs, REG_RDX);

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
    case 32 :
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
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_RM_R_MOV_R_RM, rm, off32, r, bits);
}


int mov_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_RM_R_MOV_RM_R, rm, off32, r, bits);
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
            byte_buf_append(0x0f);
            byte_buf_append(0xbe);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
        else if (src_bits == 16) {
            byte_buf_append(0x0f);
            byte_buf_append(0xbf);
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
            byte_buf_append(0x0f);
            byte_buf_append(0xb6);
            byte_buf_append(bb, 0xc0 | (dst << 3) | src);
            return 0;
        }
        else if (src_bits == 16) {
            byte_buf_append(0x0f);
            byte_buf_append(0xb6);
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
            mov_r_r(dst, rhs, 32);
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
    return op_rm_r(bb, OP_RM_R_OR, rm, off32, r, bits);
}


int pop_r64 (struct byte_buf * bb, unsigned int reg) {
    byte_buf_append(bb, 0x58 | reg);
    return 0;
}


int push_r64 (struct byte_buf * bb, unsigned int reg) {
    byte_buf_append(bb, 0x50 | reg);
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
    if (lhs != REG_ECX)
        push_r64(shift_buf, REG_ECX);
    mov_r_r(shift_buf, REG_ECX, rhs, 8);
    byte_buf_append(shift_buf, 0x48);
    byte_buf_append(shift_buf, 0xd3);
    byte_buf_append(shift_buf, 0xe0 | lhs);
    if (lhs != REG_ECX)
        pop_r64(shift_buf, REG_ECX);
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
    if (lhs != REG_ECX)
        push_r64(shift_buf, REG_ECX);
    mov_r_r(shift_buf, REG_ECX, rhs, 8);
    byte_buf_append(shift_buf, 0x48);
    byte_buf_append(shift_buf, 0xd3);
    byte_buf_append(shift_buf, 0xe8 | lhs);
    if (lhs != REG_ECX)
        pop_r64(shift_buf, REG_ECX);
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


/*************************************
* sub [REG+OFF32], REG
*************************************/
int sub_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_RM_R_SUB, rm, off32, r, bits);
}

/*************************************
* xor [REG+OFF32], REG
*************************************/
int xor_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits) {
    return op_rm_r(bb, OP_RM_R_XOR, rm, off32, r, bits);
}


/*
def store_var_reg (offset, source_register, source_size) :
    return '\x88' + chr(0x80 | (source_register << 3) | 5) + struct.pack('<l', offset)
*/

int store_var_reg (struct byte_buf * bb,
                   uint32_t offset,
                   uint8_t reg,
                   uint8_t reg_bits) {
    if (reg_bits == 1) {
        if (and_r8_imm8(bb, reg, 1))
            return mov_r_off32_r8(bb, REG_RBP, offset, reg);
        else if (and_r16_imm16(bb, reg, 1))
            return mov_r_off32_r16(bb, REG_RBP, offset, reg);
    }
    else if (reg_bits == 8) {
        if (mov_r_off32_r8(bb, REG_RBP, offset, reg))
            return mov_r_off32_r16(bb, REG_RBP, offset, reg);
    }
    else if (reg_bits == 16)
        return mov_r_off32_r16(bb, REG_RBP, offset, reg);
    else if (reg_bits == 32)
        return mov_r_off32_r32(bb, REG_RBP, offset, reg);
    else if (reg_bits== 64)
        return mov_r_off32_r64(bb, REG_RBP, offset, reg);

    return -1;
}


int get_reg_var (struct byte_buf * bb,
                 uint8_t reg,
                 uint8_t reg_bits,
                 uint32_t offset) {
    if (source_bits == 1) {
        if (mov_r8_r_off32(bb, reg, REG_RBP, offset))
            return and_r8_imm8(bb, reg, 1);
        else if (mov_r16_r_off32(bb, reg, REG_RBP, offset))
            return and_r16_imm16(bb, reg, 1);
    }
    else if (reg_bits == 8) {
        int result = mov_r8_r_off32(bb, reg, REG_RBP, offset);
        if (result)
            result = mov_r16_r_off32(bb, reg, REG_RBP, offset);
        return result;
    }
    else if (reg_bits == 16)
        return mov_r16_r_off32(bb, reg, REG_RBP, offset);
    else if (reg_bits == 32)
        return mov_r32_r_off32(bb, reg, REG_RBP, offset);
    else if (reg_bits == 64)
        return mov_r64_r_off32(bb, reg, REG_RBP, offset);

    return -1;
}

