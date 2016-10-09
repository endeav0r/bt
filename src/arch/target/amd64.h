#ifndef amd64_HEADER
#define amd64_HEADER


int store_var_reg (struct byte_buf * byte_buf,
                   uint32_t offset,
                   uint8_t reg,
                   uint8_t reg_bits);

int add_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

int and_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

int and_r8_imm8 (struct byte_buf * bb, unsigned int r8, uint8_t imm8);
int and_r16_imm16 (struct byte_buf * bb, unsigned int r16, uint16_t imm16);

int cmp_r_imm (struct byte_buf * bb,
               unsigned int r,
               uint64_t imm,
               unsigned int bits);

int cmp_r_r (struct byte_buf * bb,
             unsigned int lhs,
             unsigned int rhs,
             unsigned int bits);

int div_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int jcc (struct byte_buf * bb, unsigned int condition, int offset);

int jmp (struct byte_buf * bb, int offset);

int mod_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int mov_r_imm (struct byte_buf * bb,
               unsigned int r,
               uint64_t imm,
               unsigned int bits);

int mov_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits);

int mov_r_rm (struct byte_buf * bb,
              unsigned int r,
              unsigned int rm,
              uint32_t off32,
              unsigned int bits);

int mov_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

int mov_r64_r64 (struct byte_buf * bb, unsigned int dst, unsigned int rhs);

int movsx_r_r (struct byte_buf * bb,
               unsigned int dst,
               unsigned int dst_bits,
               unsigned int src,
               unsigned int src_bits);

int movzx_r_r (struct byte_buf * bb,
               unsigned int dst,
               unsigned int dst_bits,
               unsigned int src,
               unsigned int src_bits);

int mul_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int or_rm_r (struct byte_buf * bb,
             unsigned int rm,
             uint32_t off32,
             unsigned int r,
             unsigned int bits);

int pop_r64 (struct byte_buf * bb, unsigned int reg);

int push_r64 (struct byte_buf * bb, unsigned int reg);

int shl_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int shr_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int sub_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

int xor_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

#endif